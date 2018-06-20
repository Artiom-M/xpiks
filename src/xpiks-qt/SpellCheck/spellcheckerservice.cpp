/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spellcheckerservice.h"
#include <QThread>
#include "../Artworks/artworkmetadata.h"
#include "spellcheckitem.h"
#include "../Common/logging.h"
#include "../Common/flags.h"

namespace SpellCheck {
    SpellCheckerService::SpellCheckerService(Common::ISystemEnvironment &environment, Models::SettingsModel *settingsModel):
        m_Environment(environment),
        m_UserDictionary(environment),
        m_SettingsModel(settingsModel),
        m_RestartRequired(false),
        m_IsStopped(false)
    {
        QObject::connect(&m_UserDictionary, &UserDictionary::sizeChanged,
                         this, &SpellCheckerService::userDictWordsNumberChanged);
    }

    SpellCheckerService::~SpellCheckerService() {
        if (m_SpellCheckWorker != nullptr) {}
    }

    void SpellCheckerService::startService(const std::shared_ptr<Common::ServiceStartParams> &params) {
        if (m_SpellCheckWorker != NULL) {
            LOG_WARNING << "Attempt to start running worker";
            return;
        }

        auto coordinatorParams = std::dynamic_pointer_cast<Helpers::AsyncCoordinatorStartParams>(params);
        Helpers::AsyncCoordinator *coordinator = nullptr;
        if (coordinatorParams) { coordinator = coordinatorParams->m_Coordinator; }

        m_SpellCheckWorker = new SpellCheckWorker(getDictsRoot(),
                                                  m_Environment,
                                                  m_UserDictionary,
                                                  coordinator);
        Helpers::AsyncCoordinatorLocker locker(coordinator);
        Q_UNUSED(locker);

        QThread *thread = new QThread();
        m_SpellCheckWorker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, m_SpellCheckWorker, &SpellCheckWorker::process);
        QObject::connect(m_SpellCheckWorker, &SpellCheckWorker::stopped, thread, &QThread::quit);

        QObject::connect(m_SpellCheckWorker, &SpellCheckWorker::stopped, m_SpellCheckWorker, &SpellCheckWorker::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        QObject::connect(this, &SpellCheckerService::cancelSpellChecking,
                         m_SpellCheckWorker, &SpellCheckWorker::cancel);

        QObject::connect(m_SpellCheckWorker, &SpellCheckWorker::queueIsEmpty,
                         this, &SpellCheckerService::spellCheckQueueIsEmpty);

        QObject::connect(m_SpellCheckWorker, &SpellCheckWorker::stopped,
                         this, &SpellCheckerService::workerFinished);
        QObject::connect(m_SpellCheckWorker, &SpellCheckWorker::destroyed,
                         this, &SpellCheckerService::workerDestroyed);

        LOG_DEBUG << "starting thread...";
        thread->start();

        m_IsStopped = false;

        emit serviceAvailable(m_RestartRequired);
    }

    void SpellCheckerService::stopService() {
        LOG_DEBUG << "#";
        if (m_SpellCheckWorker != NULL) {
            m_SpellCheckWorker->stopWorking();
        } else {
            LOG_WARNING << "SpellCheckWorker is NULL";
        }

        m_IsStopped = true;
    }

    bool SpellCheckerService::isBusy() const {
        bool isBusy = (m_SpellCheckWorker != NULL) && (m_SpellCheckWorker->hasPendingJobs());
        return isBusy;
    }

    void SpellCheckerService::submitItem(Artworks::BasicKeywordsModel *itemToCheck) {
        this->submitItem(itemToCheck, Common::SpellCheckFlags::All);
    }

    void SpellCheckerService::submitItem(Artworks::BasicKeywordsModel *itemToCheck, Common::SpellCheckFlags flags) {
        if (m_SpellCheckWorker == NULL) { return; }
        if (m_IsStopped) { return; }

        Q_ASSERT(itemToCheck != nullptr);
        LOG_INFO << "flags:" << (int)flags;

        std::shared_ptr<SpellCheckItem> item(new SpellCheckItem(itemToCheck, flags, getWordAnalysisFlags()),
            [](SpellCheckItem *spi) {
            LOG_INTEGRATION_TESTS << "Delete later for single spellcheck item";
            spi->disconnect();
            spi->deleteLater();
        });
        itemToCheck->connectSignals(item.get());
        m_SpellCheckWorker->submitItem(item);
    }

    Common::ItemProcessingWorker::batch_id_t SpellCheckerService::submitItems(const std::vector<Artworks::BasicKeywordsModel *> &itemsToCheck) {
        if (m_SpellCheckWorker == NULL) { return; }
        if (m_IsStopped) { return; }

        std::vector<std::shared_ptr<ISpellCheckItem> > items;
        const size_t size = itemsToCheck.size();

        items.reserve(size);
        auto deleter = [](SpellCheckItem *spi) {
            LOG_INTEGRATION_TESTS << "Delete later for multiple spellcheck item";
            spi->disconnect();
            spi->deleteLater();
        };

        const Common::WordAnalysisFlags flags = getWordAnalysisFlags();

        for (size_t i = 0; i < size; ++i) {
            auto *itemToCheck = itemsToCheck.at(i);
            Q_ASSERT(itemToCheck != nullptr);
            std::shared_ptr<SpellCheckItem> item(new SpellCheckItem(itemToCheck, Common::SpellCheckFlags::All, flags),
                deleter);
            itemToCheck->connectSignals(item.get());
            items.emplace_back(std::dynamic_pointer_cast<ISpellCheckItem>(item));
        }

        LOG_INFO << size << "item(s)";

        m_SpellCheckWorker->submitItems(items);
        m_SpellCheckWorker->submitSeparator();
    }

    // used for spellchecking after adding a word to user dictionary
    void SpellCheckerService::submitItems(const std::vector<Artworks::BasicKeywordsModel *> &itemsToCheck,
                                          const QStringList &wordsToCheck) {
        if (m_SpellCheckWorker == NULL) { return; }
        if (m_IsStopped) { return; }

        std::vector<std::shared_ptr<ISpellCheckItem> > items;
        const size_t size = itemsToCheck.size();

        items.reserve(size);
        auto deleter = [](SpellCheckItem *spi) {
            LOG_INTEGRATION_TESTS << "Delete later for UserDict spelling item";
            spi->disconnect();
            spi->deleteLater();
        };

        const Common::WordAnalysisFlags flags = getWordAnalysisFlags();

        for (size_t i = 0; i < size; ++i) {
            auto *itemToCheck = itemsToCheck.at(i);
            std::shared_ptr<SpellCheckItem> item(new SpellCheckItem(itemToCheck, wordsToCheck, flags),
                                                 deleter);
            itemToCheck->connectSignals(item.get());
            items.emplace_back(std::dynamic_pointer_cast<ISpellCheckItem>(item));
        }

        LOG_INFO << size << "item(s)";

        m_SpellCheckWorker->submitItems(items);
        m_SpellCheckWorker->submitSeparator();
    }

    void SpellCheckerService::submitKeyword(Artworks::BasicKeywordsModel *itemToCheck, int keywordIndex) {
        Q_ASSERT(itemToCheck != nullptr);
        LOG_INFO << "index:" << keywordIndex;
        if (m_SpellCheckWorker == NULL) { return; }
        if (m_IsStopped) { return; }

        const Common::WordAnalysisFlags flags = getWordAnalysisFlags();

        std::shared_ptr<SpellCheckItem> item(new SpellCheckItem(itemToCheck, Common::SpellCheckFlags::Keywords, flags, keywordIndex),
            [](SpellCheckItem *spi) {
            LOG_INTEGRATION_TESTS << "Delete later for keyword spelling item";
            spi->deleteLater();
        });
        itemToCheck->connectSignals(item.get());
        m_SpellCheckWorker->submitFirst(item);
    }

    QStringList SpellCheckerService::suggestCorrections(const QString &word) const {
        if (m_SpellCheckWorker == NULL) {
            LOG_DEBUG << "Worker is null";
            return QStringList();
        }

        QStringList corrections = m_SpellCheckWorker->retrieveCorrections(word);
        return corrections;
    }

    void SpellCheckerService::restartWorker() {
        m_RestartRequired = true;
        stopService();
    }

    int SpellCheckerService::getUserDictWordsNumber() {
        return m_UserDictionary.size();
    }

    QStringList SpellCheckerService::getUserDictionary() const {
        return m_UserDictionary.getWords();
    }

#ifdef INTEGRATION_TESTS
    int SpellCheckerService::getSuggestionsCount() {
        return m_SpellCheckWorker->getSuggestionsCount();
    }
#endif

    void SpellCheckerService::updateUserDictionary(const QStringList &words) {
        LOG_INFO << words;
        m_UserDictionary.reset(words);
        m_UserDictionary.save();
        emit userDictUpdate(words, true);
    }

    void SpellCheckerService::cancelCurrentBatch() {
        LOG_DEBUG << "#";
        if (m_SpellCheckWorker == NULL) { return; }

        m_SpellCheckWorker->cancelPendingJobs();
    }

    bool SpellCheckerService::hasAnyPending() {
        bool hasPending = false;

        if (m_SpellCheckWorker != NULL) {
            hasPending = m_SpellCheckWorker->hasPendingJobs();
        }

        return hasPending;
    }

    void SpellCheckerService::addWordToUserDictionary(const QString &word) {
        m_UserDictionary.addWord(word);
        m_UserDictionary.save();
        emit userDictUpdate(QStringList() << word, false);
    }

    void SpellCheckerService::clearUserDictionary() {
        m_UserDictionary.clear();
        m_UserDictionary.save();
        emit userDictCleared();
    }

    void SpellCheckerService::workerFinished() {
        LOG_DEBUG << "#";
    }

    void SpellCheckerService::workerDestroyed(QObject *object) {
        Q_UNUSED(object);
        LOG_DEBUG << "#";
        m_SpellCheckWorker = NULL;

        if (m_RestartRequired) {
            LOG_INFO << "Restarting worker...";
            startService(std::shared_ptr<Common::ServiceStartParams>());
            m_RestartRequired = false;
        }
    }

    Common::WordAnalysisFlags SpellCheckerService::getWordAnalysisFlags() const {
        Common::WordAnalysisFlags result = Common::WordAnalysisFlags::None;
        if (m_SettingsModel != NULL) {
            if (m_SettingsModel->getUseSpellCheck()) {
                Common::SetFlag(result, Common::WordAnalysisFlags::Spelling);
            }

            if (m_SettingsModel->getDetectDuplicates()) {
                Common::SetFlag(result, Common::WordAnalysisFlags::Stemming);
            }
        }
        return result;
    }

    QString SpellCheckerService::getDictsRoot() const {
        QString resourcesPath;
#ifdef INTEGRATION_TESTS
        resourcesPath = STRINGIZE(DEPS_DIR);
#else
        resourcesPath = QCoreApplication::applicationDirPath();
#if defined(Q_OS_MAC)
        resourcesPath += "/../Resources/";
#endif
#endif

        resourcesPath += "/dict/";

        return QDir::cleanPath(resourcesPath);
    }
}
