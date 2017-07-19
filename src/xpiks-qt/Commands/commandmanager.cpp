/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * Xpiks is distributed under the GNU Lesser General Public License, version 3.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "commandmanager.h"
#include "../Common/defines.h"
#include "../Models/artworksrepository.h"
#include "../Models/artitemsmodel.h"
#include "../Models/imageartwork.h"
#include "../Models/combinedartworksmodel.h"
#include "../Models/artworkuploader.h"
#include "../Models/uploadinforepository.h"
#include "../Models/uploadinfo.h"
#include "../Models/artworkmetadata.h"
#include "../Encryption/secretsmanager.h"
#include "../UndoRedo/undoredomanager.h"
#include "../Models/ziparchiver.h"
#include "../Suggestion/keywordssuggestor.h"
#include "../Commands/addartworkscommand.h"
#include "../Models/filteredartitemsproxymodel.h"
#include "../Models/recentdirectoriesmodel.h"
#include "../Models/recentfilesmodel.h"
#include "../Models/metadataelement.h"
#include "../SpellCheck/spellcheckerservice.h"
#include "../Models/settingsmodel.h"
#include "../SpellCheck/spellchecksuggestionmodel.h"
#include "../MetadataIO/backupsaverservice.h"
#include "../Conectivity/telemetryservice.h"
#include "../Conectivity/updateservice.h"
#include "../Models/logsmodel.h"
#include "../Encryption/aes-qt.h"
#include "../MetadataIO/metadataiocoordinator.h"
#include "../Suggestion/locallibrary.h"
#include "../Plugins/pluginmanager.h"
#include "../Warnings/warningsservice.h"
#include "../Models/languagesmodel.h"
#include "../AutoComplete/autocompleteservice.h"
#include "../QMLExtensions/imagecachingservice.h"
#include "../Models/findandreplacemodel.h"
#include "../Models/deletekeywordsviewmodel.h"
#include "../Helpers/helpersqmlwrapper.h"
#include "../Helpers/updatehelpers.h"
#include "../Common/imetadataoperator.h"
#include "../Translation/translationmanager.h"
#include "../Translation/translationservice.h"
#include "../Models/uimanager.h"
#include "../Models/artworkproxymodel.h"
#include "../Models/sessionmanager.h"
#include "../Warnings/warningsmodel.h"
#include "../QuickBuffer/quickbuffer.h"
#include "../QuickBuffer/currenteditableartwork.h"
#include "../QuickBuffer/currenteditableproxyartwork.h"
#include "../Maintenance/maintenanceservice.h"
#include "../Helpers/asynccoordinator.h"

Commands::CommandManager::CommandManager():
    QObject(),
    m_ArtworksRepository(NULL),
    m_ArtItemsModel(NULL),
    m_FilteredItemsModel(NULL),
    m_CombinedArtworksModel(NULL),
    m_ArtworkUploader(NULL),
    m_UploadInfoRepository(NULL),
    m_WarningsService(NULL),
    m_SecretsManager(NULL),
    m_UndoRedoManager(NULL),
    m_ZipArchiver(NULL),
    m_KeywordsSuggestor(NULL),
    m_SettingsModel(NULL),
    m_RecentDirectories(NULL),
    m_RecentFiles(NULL),
    m_SpellCheckerService(NULL),
    m_SpellCheckSuggestionModel(NULL),
    m_MetadataSaverService(NULL),
    m_TelemetryService(NULL),
    m_UpdateService(NULL),
    m_LogsModel(NULL),
    m_LocalLibrary(NULL),
    m_MetadataIOCoordinator(NULL),
    m_PluginManager(NULL),
    m_LanguagesModel(NULL),
    m_ColorsModel(NULL),
    m_AutoCompleteService(NULL),
    m_ImageCachingService(NULL),
    m_DeleteKeywordsViewModel(NULL),
    m_FindAndReplaceModel(NULL),
    m_HelpersQmlWrapper(NULL),
    m_PresetsModel(NULL),
    m_PresetsModelConfig(NULL),
    m_TranslationService(NULL),
    m_TranslationManager(NULL),
    m_UIManager(NULL),
    m_ArtworkProxyModel(NULL),
    m_SessionManager(NULL),
    m_WarningsModel(NULL),
    m_QuickBuffer(NULL),
    m_MaintenanceService(NULL),
    m_ServicesInitialized(false),
    m_AfterInitCalled(false),
    m_LastCommandID(0)
{
    QObject::connect(&m_InitCoordinator, &Helpers::AsyncCoordinator::statusReported,
                     this, &Commands::CommandManager::servicesInitialized);
}

void Commands::CommandManager::InjectDependency(Models::ArtworksRepository *artworkRepository) {
    Q_ASSERT(artworkRepository != NULL); m_ArtworksRepository = artworkRepository;
    m_ArtworksRepository->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::ArtItemsModel *artItemsModel) {
    Q_ASSERT(artItemsModel != NULL); m_ArtItemsModel = artItemsModel;
    m_ArtItemsModel->setCommandManager(this);
    // do not add to availabilityListeners
}

void Commands::CommandManager::InjectDependency(Models::FilteredArtItemsProxyModel *filteredItemsModel) {
    Q_ASSERT(filteredItemsModel != NULL); m_FilteredItemsModel = filteredItemsModel;
    m_FilteredItemsModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::CombinedArtworksModel *combinedArtworksModel) {
    Q_ASSERT(combinedArtworksModel != NULL); m_CombinedArtworksModel = combinedArtworksModel;
    m_CombinedArtworksModel->setCommandManager(this);
    m_AvailabilityListeners.append(combinedArtworksModel);
}

void Commands::CommandManager::InjectDependency(Models::ArtworkUploader *artworkUploader) {
    Q_ASSERT(artworkUploader != NULL); m_ArtworkUploader = artworkUploader;
    m_ArtworkUploader->setCommandManager(this);
    m_AvailabilityListeners.append(m_ArtworkUploader);
}

void Commands::CommandManager::InjectDependency(Models::UploadInfoRepository *uploadInfoRepository) {
    Q_ASSERT(uploadInfoRepository != NULL); m_UploadInfoRepository = uploadInfoRepository;
    m_UploadInfoRepository->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Warnings::WarningsService *warningsService) {
    Q_ASSERT(warningsService != NULL); m_WarningsService = warningsService;
    m_WarningsService->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Encryption::SecretsManager *secretsManager) {
    Q_ASSERT(secretsManager != NULL); m_SecretsManager = secretsManager;
    m_SecretsManager->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(UndoRedo::UndoRedoManager *undoRedoManager) {
    Q_ASSERT(undoRedoManager != NULL); m_UndoRedoManager = undoRedoManager;
    m_UndoRedoManager->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::ZipArchiver *zipArchiver) {
    Q_ASSERT(zipArchiver != NULL); m_ZipArchiver = zipArchiver;
    m_ZipArchiver->setCommandManager(this);
    m_AvailabilityListeners.append(zipArchiver);
}

void Commands::CommandManager::InjectDependency(Suggestion::KeywordsSuggestor *keywordsSuggestor) {
    Q_ASSERT(keywordsSuggestor != NULL); m_KeywordsSuggestor = keywordsSuggestor;
    m_KeywordsSuggestor->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::SettingsModel *settingsModel) {
    Q_ASSERT(settingsModel != NULL); m_SettingsModel = settingsModel;
    m_SettingsModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::RecentDirectoriesModel *recentDirectories) {
    Q_ASSERT(recentDirectories != NULL); m_RecentDirectories = recentDirectories;
}

void Commands::CommandManager::InjectDependency(Models::RecentFilesModel *recentFiles) {
    Q_ASSERT(recentFiles != NULL); m_RecentFiles = recentFiles;
}

void Commands::CommandManager::InjectDependency(SpellCheck::SpellCheckerService *spellCheckerService) {
    Q_ASSERT(spellCheckerService != NULL); m_SpellCheckerService = spellCheckerService;
}

void Commands::CommandManager::InjectDependency(SpellCheck::SpellCheckSuggestionModel *spellCheckSuggestionModel) {
    Q_ASSERT(spellCheckSuggestionModel != NULL); m_SpellCheckSuggestionModel = spellCheckSuggestionModel;
    m_SpellCheckSuggestionModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(MetadataIO::BackupSaverService *backupSaverService) {
    Q_ASSERT(backupSaverService != NULL); m_MetadataSaverService = backupSaverService;
}

void Commands::CommandManager::InjectDependency(Conectivity::TelemetryService *telemetryService) {
    Q_ASSERT(telemetryService != NULL); m_TelemetryService = telemetryService;
}

void Commands::CommandManager::InjectDependency(Conectivity::UpdateService *updateService) {
    Q_ASSERT(updateService != NULL); m_UpdateService = updateService;
}

void Commands::CommandManager::InjectDependency(Models::LogsModel *logsModel) {
    Q_ASSERT(logsModel != NULL); m_LogsModel = logsModel;
}

void Commands::CommandManager::InjectDependency(MetadataIO::MetadataIOCoordinator *metadataIOCoordinator) {
    Q_ASSERT(metadataIOCoordinator != NULL); m_MetadataIOCoordinator = metadataIOCoordinator;
    m_MetadataIOCoordinator->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Suggestion::LocalLibrary *localLibrary) {
    Q_ASSERT(localLibrary != NULL); m_LocalLibrary = localLibrary;
    m_LocalLibrary->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Plugins::PluginManager *pluginManager) {
    Q_ASSERT(pluginManager != NULL); m_PluginManager = pluginManager;
    m_PluginManager->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::LanguagesModel *languagesModel) {
    Q_ASSERT(languagesModel != NULL); m_LanguagesModel = languagesModel;
    m_LanguagesModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(QMLExtensions::ColorsModel *colorsModel) {
    Q_ASSERT(colorsModel != NULL); m_ColorsModel = colorsModel;
}

void Commands::CommandManager::InjectDependency(AutoComplete::AutoCompleteService *autoCompleteService) {
    Q_ASSERT(autoCompleteService != NULL); m_AutoCompleteService = autoCompleteService;
}

void Commands::CommandManager::InjectDependency(QMLExtensions::ImageCachingService *imageCachingService) {
    Q_ASSERT(imageCachingService != NULL); m_ImageCachingService = imageCachingService;
}

void Commands::CommandManager::InjectDependency(Models::FindAndReplaceModel *findAndReplaceModel) {
    Q_ASSERT(findAndReplaceModel != NULL); m_FindAndReplaceModel = findAndReplaceModel;
    m_FindAndReplaceModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::DeleteKeywordsViewModel *deleteKeywordsViewModel) {
    Q_ASSERT(deleteKeywordsViewModel != NULL); m_DeleteKeywordsViewModel = deleteKeywordsViewModel;
    m_DeleteKeywordsViewModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Helpers::HelpersQmlWrapper *helpersQmlWrapper) {
    Q_ASSERT(helpersQmlWrapper != NULL); m_HelpersQmlWrapper = helpersQmlWrapper;
    m_HelpersQmlWrapper->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(KeywordsPresets::PresetKeywordsModel *presetsModel) {
    Q_ASSERT(presetsModel != NULL); m_PresetsModel = presetsModel;
    m_PresetsModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(KeywordsPresets::PresetKeywordsModelConfig *presetsModelConfig) {
    Q_ASSERT(presetsModelConfig != NULL); m_PresetsModelConfig = presetsModelConfig;
    m_PresetsModelConfig->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Translation::TranslationService *translationService) {
    Q_ASSERT(translationService != NULL); m_TranslationService = translationService;
}

void Commands::CommandManager::InjectDependency(Translation::TranslationManager *translationManager) {
    Q_ASSERT(translationManager != NULL); m_TranslationManager = translationManager;
    m_TranslationManager->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::UIManager *uiManager) {
    Q_ASSERT(uiManager != NULL); m_UIManager = uiManager;
}

void Commands::CommandManager::InjectDependency(Models::ArtworkProxyModel *artworkProxy) {
    Q_ASSERT(artworkProxy != NULL); m_ArtworkProxyModel = artworkProxy;
    m_ArtworkProxyModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::SessionManager *sessionManager) {
    Q_ASSERT(sessionManager != NULL); m_SessionManager = sessionManager;
    m_SessionManager->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Warnings::WarningsModel *warningsModel) {
    Q_ASSERT(warningsModel != NULL); m_WarningsModel = warningsModel;
}

void Commands::CommandManager::InjectDependency(QuickBuffer::QuickBuffer *quickBuffer) {
    Q_ASSERT(quickBuffer != NULL); m_QuickBuffer = quickBuffer;
    m_QuickBuffer->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Maintenance::MaintenanceService *maintenanceService) {
    Q_ASSERT(maintenanceService != NULL); m_MaintenanceService = maintenanceService;
}

std::shared_ptr<Commands::ICommandResult> Commands::CommandManager::processCommand(const std::shared_ptr<ICommandBase> &command)
{
    int id = generateNextCommandID();
    command->assignCommandID(id);
    std::shared_ptr<Commands::ICommandResult> result = command->execute(this);

    result->afterExecCallback(this);
    return result;
}

void Commands::CommandManager::addWarningsService(Common::IServiceBase<Common::IBasicArtwork, Common::WarningsCheckFlags> *service) {
    if (service != NULL) {
        // TODO: check if we don't have such checker
        m_WarningsCheckers.append(service);
    }
}

void Commands::CommandManager::recordHistoryItem(std::unique_ptr<UndoRedo::IHistoryItem> &historyItem) const {
    if (m_UndoRedoManager) {
        m_UndoRedoManager->recordHistoryItem(historyItem);
    }
}

void Commands::CommandManager::connectEntitiesSignalsSlots() const {
    if (m_SecretsManager != NULL && m_UploadInfoRepository != NULL) {
        QObject::connect(m_SecretsManager, &Encryption::SecretsManager::beforeMasterPasswordChange,
                         m_UploadInfoRepository, &Models::UploadInfoRepository::onBeforeMasterPasswordChanged);

        QObject::connect(m_SecretsManager, &Encryption::SecretsManager::afterMasterPasswordReset,
                         m_UploadInfoRepository, &Models::UploadInfoRepository::onAfterMasterPasswordReset);
    }

    if (m_ArtItemsModel != NULL && m_FilteredItemsModel != NULL) {
        QObject::connect(m_ArtItemsModel, &Models::ArtItemsModel::selectedArtworksRemoved,
                         m_FilteredItemsModel, &Models::FilteredArtItemsProxyModel::onSelectedArtworksRemoved);
    }

    if (m_SettingsModel != NULL && m_FilteredItemsModel != NULL) {
        QObject::connect(m_SettingsModel, &Models::SettingsModel::settingsUpdated,
                         m_FilteredItemsModel, &Models::FilteredArtItemsProxyModel::onSettingsUpdated);
    }

    if (m_SpellCheckerService != NULL && m_FilteredItemsModel != NULL) {
        QObject::connect(m_SpellCheckerService, &SpellCheck::SpellCheckerService::serviceAvailable,
                         m_FilteredItemsModel, &Models::FilteredArtItemsProxyModel::onSpellCheckerAvailable);
    }

    if (m_ArtworksRepository != NULL && m_ArtItemsModel != NULL) {
        QObject::connect(m_ArtworksRepository, &Models::ArtworksRepository::filesUnavailable,
                         m_ArtItemsModel, &Models::ArtItemsModel::onFilesUnavailableHandler);
    }

    if (m_ArtItemsModel != NULL && m_UndoRedoManager != NULL) {
        QObject::connect(m_UndoRedoManager, &UndoRedo::UndoRedoManager::undoStackEmpty,
                         m_ArtItemsModel, &Models::ArtItemsModel::onUndoStackEmpty);
    }

#ifndef CORE_TESTS
    if (m_SettingsModel != NULL && m_TelemetryService != NULL) {
        QObject::connect(m_SettingsModel, &Models::SettingsModel::userStatisticsChanged,
                         m_TelemetryService, &Conectivity::TelemetryService::changeReporting);
    }

    if (m_LanguagesModel != NULL && m_KeywordsSuggestor != NULL) {
        QObject::connect(m_LanguagesModel, &Models::LanguagesModel::languageChanged,
                         m_KeywordsSuggestor, &Suggestion::KeywordsSuggestor::onLanguageChanged);
    }

    if (m_HelpersQmlWrapper != NULL && m_UpdateService != NULL) {
        QObject::connect(m_UpdateService, &Conectivity::UpdateService::updateAvailable,
                         m_HelpersQmlWrapper, &Helpers::HelpersQmlWrapper::updateAvailable);

        QObject::connect(m_UpdateService, &Conectivity::UpdateService::updateDownloaded,
                         m_HelpersQmlWrapper, &Helpers::HelpersQmlWrapper::updateIsDownloaded);
    }

    if (m_PresetsModel != NULL && m_PresetsModelConfig != NULL) {
        QObject::connect(m_PresetsModelConfig, &KeywordsPresets::PresetKeywordsModelConfig::presetsUpdated,
                         m_PresetsModel, &KeywordsPresets::PresetKeywordsModel::onPresetsUpdated);
    }
#endif

    if (m_SpellCheckerService != NULL && m_ArtItemsModel != NULL) {
        QObject::connect(m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictUpdate,
                         m_ArtItemsModel, &Models::ArtItemsModel::userDictUpdateHandler);
        QObject::connect(m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictCleared,
                         m_ArtItemsModel, &Models::ArtItemsModel::userDictClearedHandler);
    }

    if (m_SpellCheckerService != NULL && m_CombinedArtworksModel != NULL) {
        QObject::connect(m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictUpdate,
                         m_CombinedArtworksModel, &Models::CombinedArtworksModel::userDictUpdateHandler);
        QObject::connect(m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictCleared,
                         m_CombinedArtworksModel, &Models::CombinedArtworksModel::userDictClearedHandler);
    }

    if (m_ArtItemsModel != NULL && m_ArtworkProxyModel != NULL) {
        QObject::connect(m_ArtItemsModel, &Models::ArtItemsModel::fileWithIndexUnavailable,
                         m_ArtworkProxyModel, &Models::ArtworkProxyModel::itemUnavailableHandler);
    }

    if (m_SpellCheckerService != NULL && m_ArtworkProxyModel != NULL) {
        QObject::connect(m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictUpdate,
                         m_ArtworkProxyModel, &Models::ArtworkProxyModel::userDictUpdateHandler);
        QObject::connect(m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictCleared,
                         m_ArtworkProxyModel, &Models::ArtworkProxyModel::userDictClearedHandler);
    }

    if (m_SpellCheckerService != NULL && m_QuickBuffer != NULL) {
        QObject::connect(m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictUpdate,
                         m_QuickBuffer, &QuickBuffer::QuickBuffer::userDictUpdateHandler);
        QObject::connect(m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictCleared,
                         m_QuickBuffer, &QuickBuffer::QuickBuffer::userDictClearedHandler);
    }

#ifdef WITH_PLUGINS
    if (m_PluginManager != NULL && m_UIManager != NULL) {
        QObject::connect(m_UIManager, &Models::UIManager::currentEditableChanged,
                         m_PluginManager, &Plugins::PluginManager::onCurrentEditableChanged);
    }

    if (m_PluginManager != NULL && m_UndoRedoManager) {
        QObject::connect(m_UndoRedoManager, &UndoRedo::UndoRedoManager::actionUndone,
                         m_PluginManager, &Plugins::PluginManager::onLastActionUndone);
    }

    if (m_PluginManager != NULL && m_PresetsModel != NULL) {
        QObject::connect(m_PresetsModel, &KeywordsPresets::PresetKeywordsModel::presetsUpdated,
                         m_PluginManager, &Plugins::PluginManager::onPresetsUpdated);
    }
#endif

    // needed for Setting Moving task because QAbstractListModel is not thread safe

    if (m_SettingsModel != NULL && m_RecentDirectories != NULL) {
        QObject::connect(m_SettingsModel, &Models::SettingsModel::recentDirectoriesUpdated,
                         m_RecentDirectories, &Models::RecentDirectoriesModel::onRecentItemsUpdated);
    }

    if (m_SettingsModel != NULL && m_RecentFiles != NULL) {
        QObject::connect(m_SettingsModel, &Models::SettingsModel::recentFilesUpdated,
                         m_RecentFiles, &Models::RecentFilesModel::onRecentItemsUpdated);
    }
}

void Commands::CommandManager::ensureDependenciesInjected() {
    Q_ASSERT(m_ArtworksRepository != NULL);
    Q_ASSERT(m_ArtItemsModel != NULL);
    Q_ASSERT(m_FilteredItemsModel != NULL);
    Q_ASSERT(m_CombinedArtworksModel != NULL);
    Q_ASSERT(m_ArtworkUploader != NULL);
    Q_ASSERT(m_UploadInfoRepository != NULL);
    Q_ASSERT(m_WarningsService != NULL);
    Q_ASSERT(m_SecretsManager != NULL);
    Q_ASSERT(m_UndoRedoManager != NULL);
    Q_ASSERT(m_ZipArchiver != NULL);
    Q_ASSERT(m_KeywordsSuggestor != NULL);
    Q_ASSERT(m_SettingsModel != NULL);
    Q_ASSERT(m_RecentDirectories != NULL);
    Q_ASSERT(m_RecentFiles != NULL);
    Q_ASSERT(m_SpellCheckerService != NULL);
    Q_ASSERT(m_SpellCheckSuggestionModel != NULL);
    Q_ASSERT(m_MetadataSaverService != NULL);
    Q_ASSERT(m_TelemetryService != NULL);
    Q_ASSERT(m_UpdateService != NULL);
    Q_ASSERT(m_LogsModel != NULL);
    Q_ASSERT(m_LocalLibrary != NULL);
    Q_ASSERT(m_MetadataIOCoordinator != NULL);
    Q_ASSERT(m_PluginManager != NULL);
    Q_ASSERT(m_LanguagesModel != NULL);
    Q_ASSERT(m_ColorsModel != NULL);
    Q_ASSERT(m_AutoCompleteService != NULL);
    Q_ASSERT(m_ImageCachingService != NULL);
    Q_ASSERT(m_FindAndReplaceModel != NULL);
    Q_ASSERT(m_DeleteKeywordsViewModel != NULL);
    Q_ASSERT(m_PresetsModel != NULL);
    Q_ASSERT(m_PresetsModelConfig != NULL);
    Q_ASSERT(m_TranslationService != NULL);
    Q_ASSERT(m_TranslationManager != NULL);
    Q_ASSERT(m_ArtworkProxyModel != NULL);
    Q_ASSERT(m_SessionManager != NULL);
    Q_ASSERT(m_WarningsModel != NULL);
    Q_ASSERT(m_QuickBuffer != NULL);
    Q_ASSERT(m_MaintenanceService != NULL);

#if !defined(INTEGRATION_TESTS) && !defined(CORE_TESTS)
    Q_ASSERT(m_UIManager != NULL);
#endif

#ifndef INTEGRATION_TESTS
    Q_ASSERT(m_HelpersQmlWrapper != NULL);
#endif
}

void Commands::CommandManager::recodePasswords(const QString &oldMasterPassword,
                                               const QString &newMasterPassword,
                                               const std::vector<std::shared_ptr<Models::UploadInfo> > &uploadInfos) const {
    if (m_SecretsManager) {
        LOG_INFO << uploadInfos.size() << "item(s)";

        for (auto &info: uploadInfos) {
            if (info->hasPassword()) {
                QString newPassword = m_SecretsManager->recodePassword(
                    info->getPassword(), oldMasterPassword, newMasterPassword);
                info->setPassword(newPassword);
            }
        }
    }
}

void Commands::CommandManager::combineArtwork(Models::ArtworkMetadata *metadata, int index) const {
    LOG_INFO << "one item with index" << index;
    if (m_CombinedArtworksModel) {
        std::vector<Models::MetadataElement> items;
        items.emplace_back(metadata, index);

        m_CombinedArtworksModel->resetModel();
        m_CombinedArtworksModel->setArtworks(items);
    }
}

void Commands::CommandManager::combineArtworks(std::vector<Models::MetadataElement> &artworks) const {
    LOG_INFO << artworks.size() << "artworks";
    if (m_CombinedArtworksModel) {
        m_CombinedArtworksModel->resetModel();
        m_CombinedArtworksModel->setArtworks(artworks);
    }
}

void Commands::CommandManager::deleteKeywordsFromArtworks(std::vector<Models::MetadataElement> &artworks) const {
    LOG_INFO << artworks.size() << "artworks";
    if (m_DeleteKeywordsViewModel != NULL) {
        m_DeleteKeywordsViewModel->setArtworks(artworks);
    }
}

void Commands::CommandManager::setArtworksForUpload(const QVector<Models::ArtworkMetadata *> &artworks) const {
    LOG_INFO << artworks.size() << "artworks";
    if (m_ArtworkUploader) {
        m_ArtworkUploader->setArtworks(artworks);
    }
}

void Commands::CommandManager::setArtworksForZipping(const QVector<Models::ArtworkMetadata *> &artworks) const {
    LOG_INFO << artworks.size() << "artworks";
    if (m_ZipArchiver) {
        m_ZipArchiver->setArtworks(artworks);
    }
}

/*virtual*/
void Commands::CommandManager::connectArtworkSignals(Models::ArtworkMetadata *metadata) const {
#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
    if (m_ArtItemsModel)
#else
    Q_ASSERT(m_ArtItemsModel != nullptr);
#endif
    {
        LOG_INTEGRATION_TESTS << "Connecting to ArtItemsModel...";

        QObject::connect(metadata, &Models::ArtworkMetadata::modifiedChanged,
                         m_ArtItemsModel, &Models::ArtItemsModel::itemModifiedChanged);

        QObject::connect(metadata, &Models::ArtworkMetadata::spellCheckErrorsChanged,
                         m_ArtItemsModel, &Models::ArtItemsModel::spellCheckErrorsChanged);

        QObject::connect(metadata, &Models::ArtworkMetadata::backupRequired,
                         m_ArtItemsModel, &Models::ArtItemsModel::artworkBackupRequested);
    }

#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
    if (m_FilteredItemsModel)
#else
    Q_ASSERT(m_FilteredItemsModel != nullptr);
#endif
    {
        LOG_INTEGRATION_TESTS << "Connecting to FilteredItemsModel...";

        QObject::connect(metadata, &Models::ArtworkMetadata::selectedChanged,
                         m_FilteredItemsModel, &Models::FilteredArtItemsProxyModel::itemSelectedChanged);
    }
}

void Commands::CommandManager::disconnectArtworkSignals(Models::ArtworkMetadata *metadata) const {
    bool disconnectStatus = false;

#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
    if (m_ArtItemsModel)
#else
    Q_ASSERT(m_ArtItemsModel != nullptr);
#endif
    {
        LOG_INTEGRATION_TESTS << "Disconnecting from ArtItemsModel...";
        disconnectStatus = QObject::disconnect(metadata, 0, m_ArtItemsModel, 0);
        if (disconnectStatus == false) { LOG_WARNING << "Disconnect returned false"; }
        disconnectStatus = QObject::disconnect(m_ArtItemsModel, 0, metadata, 0);
        if (disconnectStatus == false) { LOG_WARNING << "Disconnect returned false"; }
    }

#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
    if (m_FilteredItemsModel)
#else
    Q_ASSERT(m_FilteredItemsModel != nullptr);
#endif
    {
        LOG_INTEGRATION_TESTS << "Disconnecting from FilteredItemsModel...";
        disconnectStatus = QObject::disconnect(metadata, 0, m_FilteredItemsModel, 0);
        if (disconnectStatus == false) { LOG_WARNING << "Disconnect returned false"; }
        disconnectStatus = QObject::disconnect(m_FilteredItemsModel, 0, metadata, 0);
        if (disconnectStatus == false) { LOG_WARNING << "Disconnect returned false"; }
    }
}

void Commands::CommandManager::readMetadata(const QVector<Models::ArtworkMetadata *> &artworks,
                                            const QVector<QPair<int, int> > &rangesToUpdate) const {
#ifndef CORE_TESTS
    if (m_MetadataIOCoordinator) {
        if ((m_SettingsModel != NULL) && !m_SettingsModel->getUseExifTool()) {
            m_MetadataIOCoordinator->readMetadataExiv2(artworks, rangesToUpdate);
        } else {
            // fallback
            m_MetadataIOCoordinator->readMetadataExifTool(artworks, rangesToUpdate);
        }
    }

#else
    Q_UNUSED(artworks);
    Q_UNUSED(rangesToUpdate);
#endif
}

void Commands::CommandManager::writeMetadata(const QVector<Models::ArtworkMetadata *> &artworks, bool useBackups) const {
#ifndef CORE_TESTS
    if (m_MetadataIOCoordinator) {
        if ((m_SettingsModel != NULL) && !m_SettingsModel->getUseExifTool()) {
            m_MetadataIOCoordinator->writeMetadataExiv2(artworks);
        } else {
            // fallback
            m_MetadataIOCoordinator->writeMetadataExifTool(artworks, useBackups);
        }
    }

#else
    Q_UNUSED(artworks);
    Q_UNUSED(useBackups);
#endif
}

void Commands::CommandManager::addToLibrary(std::unique_ptr<MetadataIO::LibrarySnapshot> &artworksSnapshot) const {
    if (m_LocalLibrary) {
        m_LocalLibrary->addToLibrary(artworksSnapshot);
    }
}

void Commands::CommandManager::updateArtworks(const QVector<int> &indices) const {
    if (m_ArtItemsModel) {
        m_ArtItemsModel->updateItemsAtIndices(indices);
    }
}

void Commands::CommandManager::updateArtworks(const QVector<QPair<int, int> > &rangesToUpdate) const {
    if (m_ArtItemsModel) {
        m_ArtItemsModel->updateItemsInRanges(rangesToUpdate);
    }
}

void Commands::CommandManager::addToRecentDirectories(const QString &path) const {
    if (m_RecentDirectories) {
        m_RecentDirectories->pushItem(path);
    }
}

void Commands::CommandManager::addToRecentFiles(const QString &path) const {
    if (m_RecentFiles) {
        m_RecentFiles->pushItem(path);
    }
}

void Commands::CommandManager::addToRecentFiles(const QStringList &filenames) const {
    if (m_RecentFiles) {
        int maxFiles = m_RecentFiles->getMaxRecentItems();
        const int length = filenames.length();
        int first = qMax(0, length - maxFiles);

        for (; first < length; ++first) {
            m_RecentFiles->pushItem(filenames[first]);
        }
    }
}

void Commands::CommandManager::autoDiscoverExiftool() const {
    if (m_MetadataIOCoordinator) {
        m_MetadataIOCoordinator->autoDiscoverExiftool();
    }
}

void Commands::CommandManager::generatePreviews(const QVector<Models::ArtworkMetadata *> &items) const {
#ifndef CORE_TESTS
    if (m_SettingsModel != NULL &&
        m_SettingsModel->getAutoCacheImages() &&
        m_ImageCachingService != NULL) {
        m_ImageCachingService->generatePreviews(items);
    }

#else
    Q_UNUSED(items);
#endif
}

void Commands::CommandManager::submitKeywordForSpellCheck(Common::BasicKeywordsModel *item, int keywordIndex) const {
    Q_ASSERT(item != NULL);
    if ((m_SettingsModel != NULL) && m_SettingsModel->getUseSpellCheck() && (m_SpellCheckerService != NULL)) {
        m_SpellCheckerService->submitKeyword(item, keywordIndex);
    }
}

void Commands::CommandManager::submitForSpellCheck(const QVector<Models::ArtworkMetadata *> &items) const {
    if ((m_SettingsModel != NULL) &&
        m_SettingsModel->getUseSpellCheck() &&
        (m_SpellCheckerService != NULL) &&
        !items.isEmpty()) {
        QVector<Common::BasicKeywordsModel *> itemsToSubmit;
        int count = items.length();
        itemsToSubmit.reserve(count);

        for (int i = 0; i < count; ++i) {
            Models::ArtworkMetadata *metadata = items.at(i);
            itemsToSubmit << metadata->getBasicModel();
        }

        this->submitForSpellCheck(itemsToSubmit);
    }
}

void Commands::CommandManager::submitForSpellCheck(const QVector<Common::BasicKeywordsModel *> &items) const {
    if ((m_SettingsModel != NULL) && m_SettingsModel->getUseSpellCheck() && m_SpellCheckerService != NULL) {
        m_SpellCheckerService->submitItems(items);
    }
}

void Commands::CommandManager::submitItemForSpellCheck(Common::BasicKeywordsModel *item, Common::SpellCheckFlags flags) const {
    Q_ASSERT(item != NULL);
    if ((m_SettingsModel != NULL) && m_SettingsModel->getUseSpellCheck() && (m_SpellCheckerService != NULL)) {
        m_SpellCheckerService->submitItem(item, flags);
    }
}

void Commands::CommandManager::setupSpellCheckSuggestions(Common::IMetadataOperator *item, int index, Common::SuggestionFlags flags) {
    Q_ASSERT(item != NULL);
    if (m_SpellCheckSuggestionModel) {
        m_SpellCheckSuggestionModel->setupModel(item, index, flags);
        reportUserAction(Conectivity::UserAction::SpellSuggestions);
    }
}

void Commands::CommandManager::setupSpellCheckSuggestions(std::vector<std::pair<Common::IMetadataOperator *, int> > &itemPairs, Common::SuggestionFlags flags) {
    if (m_SpellCheckSuggestionModel) {
        m_SpellCheckSuggestionModel->setupModel(itemPairs, flags);
        reportUserAction(Conectivity::UserAction::SpellSuggestions);
    }
}

void Commands::CommandManager::submitForSpellCheck(const QVector<Common::BasicKeywordsModel *> &items,
                                                   const QStringList &wordsToCheck) const {
    if ((m_SettingsModel != NULL) && m_SettingsModel->getUseSpellCheck() && m_SpellCheckerService != NULL) {
        m_SpellCheckerService->submitItems(items, wordsToCheck);
    }
}

void Commands::CommandManager::submitKeywordsForWarningsCheck(Models::ArtworkMetadata *item) const {
    Q_ASSERT(item != NULL);
    this->submitForWarningsCheck(item, Common::WarningsCheckFlags::Keywords);
}

void Commands::CommandManager::submitForWarningsCheck(Models::ArtworkMetadata *item, Common::WarningsCheckFlags flags) const {
    Q_ASSERT(item != NULL);

    if (m_WarningsService != NULL) {
        m_WarningsService->submitItem(item, flags);
    }

    int count = m_WarningsCheckers.length();

    LOG_INTEGRATION_TESTS << count << "checkers available";

    for (int i = 0; i < count; ++i) {
        Common::IServiceBase<Common::IBasicArtwork, Common::WarningsCheckFlags> *checker = m_WarningsCheckers.at(i);
        if (checker->isAvailable()) {
            checker->submitItem(item, flags);
        }
    }
}

void Commands::CommandManager::submitForWarningsCheck(const QVector<Models::ArtworkMetadata *> &items) const {
    if (m_WarningsService != NULL) {
        m_WarningsService->submitItems(items);
    }

    if (!m_WarningsCheckers.isEmpty()) {
        QVector<Common::IBasicArtwork *> itemsToSubmit;
        int count = items.length();
        itemsToSubmit.reserve(count);

        for (int i = 0; i < count; ++i) {
            itemsToSubmit << items.at(i);
        }

        this->submitForWarningsCheck(itemsToSubmit);
    }
}

void Commands::CommandManager::submitForWarningsCheck(const QVector<Common::IBasicArtwork *> &items) const {
    int count = m_WarningsCheckers.length();

    for (int i = 0; i < count; ++i) {
        Common::IServiceBase<Common::IBasicArtwork, Common::WarningsCheckFlags> *checker = m_WarningsCheckers.at(i);
        if (checker->isAvailable()) {
            checker->submitItems(items);
        }
    }
}

void Commands::CommandManager::saveArtworkBackup(Models::ArtworkMetadata *metadata) const {
    Q_ASSERT(metadata != NULL);
    if ((m_SettingsModel != NULL) && m_SettingsModel->getSaveBackups() && m_MetadataSaverService != NULL) {
        m_MetadataSaverService->saveArtwork(metadata);
    }
}

void Commands::CommandManager::saveArtworksBackups(const QVector<Models::ArtworkMetadata *> &artworks) const {
    if ((m_SettingsModel != NULL) && m_SettingsModel->getSaveBackups() && m_MetadataSaverService != NULL) {
        m_MetadataSaverService->saveArtworks(artworks);
    }
}

void Commands::CommandManager::reportUserAction(Conectivity::UserAction userAction) const {
#ifndef CORE_TESTS
    if (m_TelemetryService) {
        m_TelemetryService->reportAction(userAction);
    }
#else
    Q_UNUSED(userAction);
#endif
}

void Commands::CommandManager::cleanupLocalLibraryAsync() const {
    if (m_LocalLibrary) {
        m_MaintenanceService->cleanupLocalLibrary(m_LocalLibrary);
    }
}

void Commands::CommandManager::afterConstructionCallback() {
    if (m_AfterInitCalled) {
        LOG_WARNING << "Attempt to call afterConstructionCallback() second time";
        return;
    }

    const int waitSeconds = 5;
    Helpers::AsyncCoordinatorStarter defferedStarter(&m_InitCoordinator, waitSeconds);
    Q_UNUSED(defferedStarter);

    m_AfterInitCalled = true;
    std::shared_ptr<Common::ServiceStartParams> emptyParams;
    std::shared_ptr<Common::ServiceStartParams> coordinatorParams(
                new Helpers::AsyncCoordinatorStartParams(&m_InitCoordinator));

    m_MaintenanceService->startService();

#ifndef CORE_TESTS
    m_ImageCachingService->startService(coordinatorParams);
#endif
    m_SpellCheckerService->startService(coordinatorParams);
    m_WarningsService->startService(emptyParams);
    m_MetadataSaverService->startSaving();
    m_AutoCompleteService->startService(coordinatorParams);
    m_TranslationService->startService(coordinatorParams);

    QCoreApplication::processEvents();

#ifndef CORE_TESTS
    const QString reportingEndpoint =
        QLatin1String(
            "cc39a47f60e1ed812e2403b33678dd1c529f1cc43f66494998ec478a4d13496269a3dfa01f882941766dba246c76b12b2a0308e20afd84371c41cf513260f8eb8b71f8c472cafb1abf712c071938ec0791bbf769ab9625c3b64827f511fa3fbb");
    QString endpoint = Encryption::decodeText(reportingEndpoint, "reporting");
    m_TelemetryService->setEndpoint(endpoint);

    m_TelemetryService->startReporting();
    m_UpdateService->startChecking();
    m_ArtworkUploader->initializeStocksList(&m_InitCoordinator);
    m_WarningsService->initWarningsSettings();
    m_TranslationManager->initializeDictionaries(&m_InitCoordinator);
    m_PresetsModelConfig->initializeConfigs(&m_InitCoordinator);
#endif

#ifdef Q_OS_MAC
    if (m_SettingsModel->getUseExifTool()) {
        QCoreApplication::processEvents();
        m_MetadataIOCoordinator->autoDiscoverExiftool();
    }
#endif

    executeMaintenanceJobs();

    readSession();
}

void Commands::CommandManager::afterInnerServicesInitialized() {
    LOG_DEBUG << "#";

#ifndef CORE_TESTS
#ifdef WITH_PLUGINS
    m_PluginManager->loadPlugins();
#endif
#endif

    afterReadSession();
}

void Commands::CommandManager::executeMaintenanceJobs() {
    m_MaintenanceService->loadLocalLibrary(m_LocalLibrary);

#if !defined(CORE_TESTS) && !defined(INTEGRATION_TESTS)
    m_MaintenanceService->moveSettings(m_SettingsModel);

    m_MaintenanceService->cleanupLogs();
    m_MaintenanceService->cleanupUpdatesArtifacts();
#endif
}

void Commands::CommandManager::readSession() {
    LOG_DEBUG << "#";

    Q_ASSERT(m_SettingsModel != nullptr);
    if (!m_SettingsModel->getSaveSession()) {
        return;
    }

    m_SessionManager->restoreFromFile();
}

int Commands::CommandManager::afterReadSession() {
    LOG_DEBUG << "#";

    if (!m_SettingsModel->getSaveSession()) {
        return 0;
    }

    bool autoFindVectors = m_SettingsModel->getAutoFindVectors();
    auto filenames = m_SessionManager->getFilenames();
    auto vectors = m_SessionManager->getVectors();

    if (filenames.empty()) {
        LOG_WARNING << "Session was empty";
        return 0;
    }

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, vectors, autoFindVectors));
    std::shared_ptr<Commands::ICommandResult> result = processCommand(addArtworksCommand);
    std::shared_ptr<Commands::AddArtworksCommandResult> addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);

    int newFilesCount = addArtworksResult->m_NewFilesAdded;
    return newFilesCount;
}

#ifdef INTEGRATION_TESTS
int Commands::CommandManager::restoreSession() {
    readSession();

    return afterReadSession();
}
#endif

void Commands::CommandManager::saveSession() const {
    LOG_DEBUG << "#";

    Q_ASSERT(m_SettingsModel != nullptr);
    if (!m_SettingsModel->getSaveSession()) {
        return;
    }

    auto artworkList = m_ArtItemsModel->getArtworkList();
    MetadataIO::SessionSnapshot sessionSnapshot(artworkList);
    auto snapshot = sessionSnapshot.getSnapshot();

    m_SessionManager->saveToFile(snapshot);
}

void Commands::CommandManager::saveSessionInBackground() {
    LOG_DEBUG << "#";

    if (!m_SettingsModel || !m_SettingsModel->getSaveSession()) {
        return;
    }

    auto artworkList = m_ArtItemsModel->getArtworkList();
    std::unique_ptr<MetadataIO::SessionSnapshot> sessionSnapshot(new MetadataIO::SessionSnapshot(artworkList));

    m_MaintenanceService->saveSession(sessionSnapshot, m_SessionManager);
}

void Commands::CommandManager::beforeDestructionCallback() const {
    LOG_DEBUG << "Shutting down...";
    if (!m_AfterInitCalled) {
        return;
    }

    saveSession();
    m_ArtworksRepository->stopListeningToUnavailableFiles();

    m_ArtItemsModel->disconnect();
    m_ArtItemsModel->deleteAllItems();
    m_FilteredItemsModel->disconnect();

#ifndef CORE_TESTS
    m_ImageCachingService->stopService();
    m_UpdateService->stopChecking();
#endif
    m_SpellCheckerService->stopService();
    m_WarningsService->stopService();
    m_MetadataSaverService->stopSaving();
    m_AutoCompleteService->stopService();
    m_TranslationService->stopService();

#ifndef CORE_TESTS

#ifdef WITH_PLUGINS
    m_PluginManager->unloadPlugins();
#endif

    // we have a second for important stuff
    m_TelemetryService->reportAction(Conectivity::UserAction::Close);
    m_TelemetryService->stopReporting();

    m_LogsModel->stopLogging();
#endif

    m_MaintenanceService->stopService();

    m_SettingsModel->syncronizeSettings();
}

void Commands::CommandManager::requestCloseApplication() const {
    if (m_HelpersQmlWrapper != NULL) {
        m_HelpersQmlWrapper->requestCloseApplication();
    }
}

void Commands::CommandManager::restartSpellChecking() {
    if (m_SpellCheckerService) {
        m_SpellCheckerService->restartWorker();
    }
}

#ifndef CORE_TESTS
void Commands::CommandManager::autoCompleteKeyword(const QString &keyword, QObject *notifyObject) const {
    if ((m_SettingsModel != NULL) && m_SettingsModel->getUseAutoComplete() && (m_AutoCompleteService != NULL)) {
        m_AutoCompleteService->findKeywordCompletions(keyword, notifyObject);
    }
}
#endif

void Commands::CommandManager::removeUnavailableFiles() {
    LOG_DEBUG << "#";

    m_CombinedArtworksModel->generateAboutToBeRemoved();
    m_ArtItemsModel->generateAboutToBeRemoved();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    int size = m_AvailabilityListeners.size();
    for (int i = 0; i < size; ++i) {
        m_AvailabilityListeners[i]->removeUnavailableItems();
    }

    m_ArtItemsModel->removeUnavailableItems();

#ifndef CORE_TESTS
    m_UndoRedoManager->discardLastAction();
#endif

    if (m_ArtworksRepository->canPurgeUnavailableFiles()) {
        m_ArtworksRepository->purgeUnavailableFiles();
    } else {
        LOG_INFO << "Unavailable files purging postponed";
    }
}

#ifdef INTEGRATION_TESTS
void Commands::CommandManager::cleanup() {
    LOG_INTEGRATION_TESTS << "#";
    m_CombinedArtworksModel->resetModel();
    m_ZipArchiver->resetArtworks();
    m_ZipArchiver->resetModel();
    m_ArtworkUploader->resetArtworks();
    m_ArtworkUploader->resetModel();
    m_ArtworksRepository->resetEverything();
    m_ArtItemsModel->deleteAllItems();
    m_SettingsModel->resetToDefault();
    m_SpellCheckerService->clearUserDictionary();
}
#endif

void Commands::CommandManager::servicesInitialized(int status) {
    LOG_DEBUG << "#";
    Q_ASSERT(m_ServicesInitialized == false);

    Helpers::AsyncCoordinator::CoordinationStatus coordStatus = (Helpers::AsyncCoordinator::CoordinationStatus)status;

    if (m_ServicesInitialized == false) {
        m_ServicesInitialized = true;

        if (coordStatus == Helpers::AsyncCoordinator::AllDone ||
                coordStatus == Helpers::AsyncCoordinator::Timeout) {
            this->afterInnerServicesInitialized();
        }
    }
}

void Commands::CommandManager::registerCurrentItem(const Models::MetadataElement &metadataElement) {
    if (m_UIManager != nullptr) {
        std::shared_ptr<QuickBuffer::ICurrentEditable> currentItem(new QuickBuffer::CurrentEditableArtwork(
                                                                       metadataElement.getOrigin(),
                                                                       metadataElement.getOriginalIndex(),
                                                                       this));
        m_UIManager->registerCurrentItem(currentItem);
    }
}

void Commands::CommandManager::registerCurrentItem(Models::ArtworkProxyBase *artworkProxy) const {
    if (m_UIManager != nullptr) {
        std::shared_ptr<QuickBuffer::ICurrentEditable> currentItem(new QuickBuffer::CurrentEditableProxyArtwork(artworkProxy));
        m_UIManager->registerCurrentItem(currentItem);
    }
}

void Commands::CommandManager::clearCurrentItem() const {
    LOG_DEBUG << "#";
    if (m_UIManager != nullptr) {
        m_UIManager->clearCurrentItem();
    }
}
