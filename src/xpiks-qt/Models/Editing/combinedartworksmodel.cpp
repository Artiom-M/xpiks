/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "combinedartworksmodel.h"
#include <Helpers/indiceshelper.h>
#include <Commands/Editing/modifyartworkscommand.h>
#include <Commands/Editing/editartworkstemplate.h>
#include <Commands/commandmanager.h>
#include <Suggestion/keywordssuggestor.h>
#include <Artworks/artworkmetadata.h>
#include <Artworks/artworkelement.h>
#include <Common/defines.h>
#include <QMLExtensions/uicommandid.h>
#include <KeywordsPresets/ipresetsmanager.h>
#include <Models/Editing/currenteditableproxyartwork.h>

#define MAX_EDITING_PAUSE_RESTARTS 12

namespace Models {
    CombinedArtworksModel::CombinedArtworksModel(Commands::ICommandManager &commandManager,
                                                 KeywordsPresets::IPresetsManager &presetsManager,
                                                 QObject *parent):
        ArtworksViewModel(parent),
        ArtworkProxyBase(),
        Common::DelayedActionEntity(1000, MAX_EDITING_PAUSE_RESTARTS),
        m_CommandManager(commandManager),
        m_PresetsManager(presetsManager),
        m_CommonKeywordsModel(m_HoldPlaceholder, this),
        m_EditFlags(Common::ArtworkEditFlags::None),
        m_ModifiedFlags(0)
    {
        m_CommonKeywordsModel.setSpellCheckInfo(&m_SpellCheckInfo);

        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::titleSpellingChanged,
                         this, &CombinedArtworksModel::onTitleSpellingChanged);
        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::descriptionSpellingChanged,
                         this, &CombinedArtworksModel::onDescriptionSpellingChanged);

        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::completionsAvailable,
                         this, &CombinedArtworksModel::completionsAvailable);

        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::afterSpellingErrorsFixed,
                         this, &CombinedArtworksModel::spellCheckErrorsFixedHandler);

        QObject::connect(this, &CombinedArtworksModel::editingPaused,
                         this, &CombinedArtworksModel::onEditingPaused);

        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::descriptionSpellingChanged,
                         this, &CombinedArtworksModel::descriptionSpellingChanged);
        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::titleSpellingChanged,
                         this, &CombinedArtworksModel::titleSpellingChanged);
        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::keywordsSpellingChanged,
                         this, &CombinedArtworksModel::keywordsSpellingChanged);
    }

    void CombinedArtworksModel::setArtworks(Artworks::ArtworksSnapshot const &artworks) {
        ArtworksViewModel::setArtworks(artworks);

        recombineArtworks();

        if (getArtworksCount() == 1) {
            enableAllFields();
        }
    }

    void CombinedArtworksModel::recombineArtworks() {
        LOG_INFO << getArtworksCount() << "artwork(s)";

        LOG_DEBUG << "Before recombine description:" << getDescription();
        LOG_DEBUG << "Before recombine title:" << getTitle();
        LOG_DEBUG << "Before recombine keywords:" << getKeywordsString();

        if (isEmpty()) { return; }

        if (getArtworksCount() == 1) {
            assignFromOneArtwork();
        } else {
            assignFromManyArtworks();
        }

        LOG_DEBUG << "After recombine description:" << getDescription();
        LOG_DEBUG << "After recombine title:" << getTitle();
        LOG_DEBUG << "After recombine keywords:" << getKeywordsString();

        sendMessage(&m_CommonKeywordsModel);
    }

    void CombinedArtworksModel::setDescription(const QString &value) {
        if (doSetDescription(value)) {
            signalDescriptionChanged();
            setDescriptionModified(true);
        }
    }

    void CombinedArtworksModel::setTitle(const QString &value) {
        if (doSetTitle(value)) {
            signalTitleChanged();
            setTitleModified(true);
        }
    }

    void CombinedArtworksModel::setKeywords(const QStringList &keywords) {
        ArtworkProxyBase::setKeywords(keywords);
        setKeywordsModified(true);
    }

    void CombinedArtworksModel::setChangeDescription(bool value) {
        LOG_INFO << value;
        auto flag = Common::ArtworkEditFlags::EditDescription;
        if (Common::HasFlag(m_EditFlags, flag) != value) {
            Common::ApplyFlag(m_EditFlags, value, flag);
            emit changeDescriptionChanged();
        }
    }

    void CombinedArtworksModel::setChangeTitle(bool value) {
        LOG_INFO << value;
        auto flag = Common::ArtworkEditFlags::EditTitle;
        if (Common::HasFlag(m_EditFlags, flag) != value) {
            Common::ApplyFlag(m_EditFlags, value, flag);
            emit changeTitleChanged();
        }
    }

    void CombinedArtworksModel::setChangeKeywords(bool value) {
        LOG_INFO << value;
        auto flag = Common::ArtworkEditFlags::EditKeywords;
        if (Common::HasFlag(m_EditFlags, flag) != value) {
            Common::ApplyFlag(m_EditFlags, value, flag);
            emit changeKeywordsChanged();
        }
    }

    void CombinedArtworksModel::setAppendKeywords(bool value) {
        LOG_INFO << value;
        auto flag = Common::ArtworkEditFlags::AppendKeywords;
        if (Common::HasFlag(m_EditFlags, flag) != value) {
            Common::ApplyFlag(m_EditFlags, value, flag);
            emit appendKeywordsChanged();
        }
    }

#ifdef CORE_TESTS
    QStringList CombinedArtworksModel::getKeywords() {
        return m_CommonKeywordsModel.getKeywords();
    }
#endif

    void CombinedArtworksModel::editKeyword(int index, const QString &replacement) {
        if (doEditKeyword(index, replacement)) {
            setKeywordsModified(true);
        }
    }

    QString CombinedArtworksModel::removeKeywordAt(int keywordIndex) {
        QString keyword;
        if (doRemoveKeywordAt(keywordIndex, keyword)) {
            setKeywordsModified(true);
        }
        return keyword;
    }

    void CombinedArtworksModel::removeLastKeyword() {
        QString keyword;
        if (doRemoveLastKeyword(keyword)) {
            setKeywordsModified(true);
        }
    }

    bool CombinedArtworksModel::moveKeyword(int from, int to) {
        return doMoveKeyword(from, to);
    }

    bool CombinedArtworksModel::appendKeyword(const QString &keyword) {
        const bool added = doAppendKeyword(keyword);
        if (added) {
            setKeywordsModified(true);
        }

        return added;
    }

    void CombinedArtworksModel::pasteKeywords(const QStringList &keywords) {
        if (doAppendKeywords(keywords) > 0) {
            setKeywordsModified(true);
        }
    }

    void CombinedArtworksModel::saveEdits() {
        LOG_INFO << "edit flags:" << (int)m_EditFlags << "modified flags:" << m_ModifiedFlags;
        bool needToSave = false;

        if (getChangeTitle() ||
            getChangeDescription() ||
            getChangeKeywords()) {
            needToSave = getArtworksCount() > 1;
            needToSave = needToSave || (getChangeKeywords() && areKeywordsModified());
            needToSave = needToSave || isSpellingFixed();
            needToSave = needToSave || (getChangeTitle() && isTitleModified());
            needToSave = needToSave || (getChangeDescription() && isDescriptionModified());
        }

        if (needToSave) {
            using namespace Commands;
            Artworks::ArtworksSnapshot snapshot;
            snapshot.copyFrom(getSnapshot());
            m_CommandManager.processCommand(
                        std::make_shared<ModifyArtworksCommand>(
                            std::move(snapshot),
                            std::make_shared<EditArtworksTemplate>(
                                m_CommonKeywordsModel.getTitle(),
                                m_CommonKeywordsModel.getDescription(),
                                m_CommonKeywordsModel.getKeywords(),
                                m_EditFlags)));
        } else {
            LOG_DEBUG << "nothing to save";
        }
    }

    void CombinedArtworksModel::clearKeywords() {
        if (doClearKeywords()) {
            setKeywordsModified(true);
        }
    }

    void CombinedArtworksModel::assignFromSelected() {
        LOG_DEBUG << "#";
        recombineArtworks([](const Artworks::ArtworkElement *item) { return item->getIsSelected(); });

        LOG_DEBUG << "After recombine description:" << getDescription();
        LOG_DEBUG << "After recombine title:" << getTitle();
        LOG_DEBUG << "After recombine keywords:" << getKeywordsString();

        sendMessage(&m_CommonKeywordsModel);
    }

    void CombinedArtworksModel::plainTextEdit(const QString &rawKeywords, bool spaceIsSeparator) {
        doPlainTextEdit(rawKeywords, spaceIsSeparator);
        setKeywordsModified(true);
    }

    bool CombinedArtworksModel::hasTitleWordSpellError(const QString &word) {
        return getHasTitleWordSpellError(word);
    }

    bool CombinedArtworksModel::hasDescriptionWordSpellError(const QString &word) {
        return getHasDescriptionWordSpellError(word);
    }

    void CombinedArtworksModel::expandPreset(int keywordIndex, unsigned int presetID) {
        if (doExpandPreset(keywordIndex, (KeywordsPresets::ID_t)presetID, m_PresetsManager)) {
            setKeywordsModified(true);
        }
    }

    void CombinedArtworksModel::expandLastKeywordAsPreset() {
        doExpandLastKeywordAsPreset(m_PresetsManager);
    }

    void CombinedArtworksModel::addPreset(unsigned int presetID) {
        if (doAppendPreset((KeywordsPresets::ID_t)presetID, m_PresetsManager)) {
            setKeywordsModified(true);
        }
    }

    void CombinedArtworksModel::copyToQuickBuffer() {
        LOG_DEBUG << "#";
        sendMessage(
                    QuickBufferMessage(
                        m_CommonKeywordsModel.getTitle(),
                        m_CommonKeywordsModel.getDescription(),
                        m_CommonKeywordsModel.getKeywords(),
                        false));
    }

    bool CombinedArtworksModel::acceptCompletionAsPreset(AutoComplete::ICompletionSource &completionSource, int completionID) {
        LOG_DEBUG << completionID;
        const bool accepted = doAcceptCompletionAsPreset(completionID, completionSource, m_PresetsManager);
        m_CommonKeywordsModel.notifyCompletionAccepted(accepted, completionID);
        return accepted;
    }

    void CombinedArtworksModel::enableAllFields() {
        setChangeDescription(true);
        setChangeTitle(true);
        setChangeKeywords(true);
    }

    void CombinedArtworksModel::assignFromOneArtwork() {
        LOG_DEBUG << "#";
        Q_ASSERT(getArtworksCount() == 1);
        Artworks::ArtworkMetadata *artwork = accessItem(0)->getArtworkMetadata();

        if (!isDescriptionModified()) {
            initDescription(artwork->getDescription());
        }

        if (!isTitleModified()) {
            initTitle(artwork->getTitle());
        }

        if (!areKeywordsModified()) {
            initKeywords(artwork->getKeywords());
        }
    }

    void CombinedArtworksModel::assignFromManyArtworks() {
        recombineArtworks([](const Artworks::ArtworkElement *) { return true; });
    }

    void CombinedArtworksModel::recombineArtworks(std::function<bool (const Artworks::ArtworkElement *)> pred) {
        LOG_DEBUG << "#";

        bool descriptionsDiffer = false;
        bool titleDiffer = false;
        bool anyDifferent = false;
        QString description, title;
        QSet<QString> commonKeywords, unitedKeywords;
        QStringList firstItemKeywords;
        int firstItemKeywordsCount = 0;
        int firstNonEmptyIndex = 0;
        Artworks::ArtworkMetadata *firstNonEmpty = nullptr;

        if (findNonEmptyData(pred, firstNonEmptyIndex, firstNonEmpty)) {
            description = firstNonEmpty->getDescription();
            title = firstNonEmpty->getTitle();
            firstItemKeywords = firstNonEmpty->getKeywords();
            // preserve case with List to Set convertion
            auto firstSet = firstItemKeywords.toSet();
            commonKeywords.unite(firstSet);
            firstItemKeywordsCount = firstSet.count();
        }

        processArtworks(pred,
                        [&](size_t index, Artworks::ArtworkMetadata *metadata) {
            if ((int)index == firstNonEmptyIndex) { return; }

            QString currDescription = metadata->getDescription();
            QString currTitle = metadata->getTitle();
            descriptionsDiffer = descriptionsDiffer || ((!currDescription.isEmpty()) && (description != currDescription));
            titleDiffer = titleDiffer || ((!currTitle.isEmpty()) && (title != currTitle));

            // preserve case with List to Set convertion
            auto currentSet = metadata->getKeywords().toSet();

            if (!currentSet.isEmpty()) {
                commonKeywords.intersect(currentSet);

                // used to detect if all items have same keywords
                if ((currentSet.count() == firstItemKeywordsCount) &&
                    (unitedKeywords.count() <= firstItemKeywordsCount)) {
                    unitedKeywords.unite(currentSet);
                } else {
                    anyDifferent = true;
                }
            }
        });

        if (!isEmpty()) {
            if (descriptionsDiffer) {
                description = "";
            }

            if (titleDiffer) {
                title = "";
            }

            initDescription(description);
            initTitle(title);

            if (!areKeywordsModified()) {
                if ((!anyDifferent) && unitedKeywords.subtract(commonKeywords).isEmpty()) {
                    // all keywords are the same
                    initKeywords(firstItemKeywords);
                } else {
                    initKeywords(commonKeywords.toList());
                }
            }
        }
    }

    bool CombinedArtworksModel::findNonEmptyData(std::function<bool (const Artworks::ArtworkElement *)> pred,
                                                 int &index,
                                                 Artworks::ArtworkMetadata *&artworkMetadata) {
        bool found = false, foundOther = false;
        int nonEmptyKeywordsIndex = -1, nonEmptyOtherIndex = -1;
        Artworks::ArtworkMetadata *nonEmptyKeywordsArtwork = nullptr;
        Artworks::ArtworkMetadata *nonEmptyOtherMetadata = nullptr;

        processArtworksEx(pred,
                        [&](size_t index, Artworks::ArtworkMetadata *artworks) -> bool {
            if (!artworks->areKeywordsEmpty()) {
                nonEmptyKeywordsIndex = (int)index;
                nonEmptyKeywordsArtwork = artworks;
                found = true;
            } else {
                if (!foundOther) {
                    if (!artworks->getDescription().trimmed().isEmpty() ||
                            !artworks->getTitle().trimmed().isEmpty()) {
                        nonEmptyOtherIndex = (int)index;
                        nonEmptyOtherMetadata = artworks;
                        foundOther = true;
                    }
                }
            }

            bool shouldContinue = !found;
            return shouldContinue;
        });

        if (found) {
            LOG_INFO << "Found artwork with non-empty keywords at" << nonEmptyKeywordsIndex;
            artworkMetadata = nonEmptyKeywordsArtwork;
            index = nonEmptyKeywordsIndex;
        } else if (foundOther) {
            LOG_INFO << "Found artwork with non-empty other data at" << nonEmptyOtherIndex;
            artworkMetadata = nonEmptyOtherMetadata;
            index = nonEmptyOtherIndex;
        } else {
            LOG_WARNING << "All artworks seem to be blank";
        }

        return found || foundOther;
    }

    void CombinedArtworksModel::onDescriptionSpellingChanged() {
        emit descriptionChanged();
    }

    void CombinedArtworksModel::onTitleSpellingChanged() {
        emit titleChanged();
    }

    void CombinedArtworksModel::spellCheckErrorsFixedHandler() {
        setSpellingFixed(true);
        // for possible squeezing after replace
        emit keywordsCountChanged();
    }

    void CombinedArtworksModel::generateAboutToBeRemoved() {
        LOG_DEBUG << "#";
        m_CommonKeywordsModel.notifyAboutToBeRemoved();
    }

    void CombinedArtworksModel::userDictUpdateHandler(const QStringList &keywords, bool overwritten) {
        LOG_DEBUG << "#";
        doHandleUserDictChanged(keywords, overwritten);
    }

    void CombinedArtworksModel::userDictClearedHandler() {
        LOG_DEBUG << "#";
        doHandleUserDictCleared();
    }

    void CombinedArtworksModel::onEditingPaused() {
        LOG_INTEGR_TESTS_OR_DEBUG << "#";
        submitForInspection();
    }

    void CombinedArtworksModel::doJustEdited() {
        justChanged();
    }

    void CombinedArtworksModel::submitForInspection() {
        sendMessage(&m_CommonKeywordsModel);
    }

    Common::ID_t CombinedArtworksModel::getSpecialItemID() {
        return SPECIAL_ID_COMBINED_MODEL;
    }

    bool CombinedArtworksModel::doRemoveSelectedArtworks() {
        LOG_DEBUG << "#";
        bool anyRemoved = ArtworksViewModel::doRemoveSelectedArtworks();
        if (anyRemoved) {
            if (!isEmpty()) {
                recombineArtworks();
            }
        }

        return anyRemoved;
    }

    void CombinedArtworksModel::doResetModel() {
        LOG_DEBUG << "#";
        ArtworksViewModel::doResetModel();

        m_SpellCheckInfo.clear();

        // TEMPORARY (enable everything on initial launch) --
        m_ModifiedFlags = 0;
        m_EditFlags = Common::ArtworkEditFlags::None;
        enableAllFields();
        // TEMPORARY (enable everything on initial launch) --

        initDescription("");
        initTitle("");
        initKeywords(QStringList());

        emit clearCurrentEditable();
    }

    void CombinedArtworksModel::doOnTimer() {
        emit editingPaused();
    }

    bool CombinedArtworksModel::removeUnavailableItems() {
        LOG_DEBUG << "#";
        bool anyRemoved = ArtworksViewModel::removeUnavailableItems();
        if (anyRemoved) {
            if (!isEmpty()) {
                recombineArtworks();
            }
        }

        return anyRemoved;
    }
}
