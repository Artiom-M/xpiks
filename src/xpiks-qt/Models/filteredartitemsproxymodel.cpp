/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2015 Taras Kushnir <kushnirTV@gmail.com>
 *
 * Xpiks is distributed under the GNU General Public License, version 3.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "filteredartitemsproxymodel.h"
#include <QDir>
#include "artitemsmodel.h"
#include "artworkmetadata.h"
#include "artworksrepository.h"
#include "artiteminfo.h"
#include "settingsmodel.h"
#include "../Commands/commandmanager.h"
#include "../Commands/combinededitcommand.h"
#include "../Common/flags.h"
#include "../SpellCheck/ispellcheckable.h"

namespace Models {
    FilteredArtItemsProxyModel::FilteredArtItemsProxyModel(QObject *parent) :
        QSortFilterProxyModel(parent),
        Common::BaseEntity(),
        m_SelectedArtworksCount(0)
    {
    }

    void FilteredArtItemsProxyModel::setSearchTerm(const QString &value) {
        if (value != m_SearchTerm) {
            m_SearchTerm = value;
            emit searchTermChanged(value);
            forceUnselectAllItems();
        }

        invalidateFilter();
        emit afterInvalidateFilter();
    }

    int FilteredArtItemsProxyModel::getOriginalIndex(int index) {
        QModelIndex originalIndex = mapToSource(this->index(index, 0));
        int row = originalIndex.row();
        return row;
    }

    void FilteredArtItemsProxyModel::selectDirectory(int directoryIndex) {
        QVector<int> directoryItems;
        int size = this->rowCount();
        directoryItems.reserve(size);

        ArtItemsModel *artItemsModel = getArtItemsModel();
        const ArtworksRepository *artworksRepository = m_CommandManager->getArtworksRepository();
        const QString &directory = artworksRepository->getDirectory(directoryIndex);
        QDir dir(directory);
        QString directoryAbsolutePath = dir.absolutePath();

        for (int row = 0; row < size; ++row) {
            QModelIndex proxyIndex = this->index(row, 0);
            QModelIndex originalIndex = this->mapToSource(proxyIndex);

            int index = originalIndex.row();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(index);
            Q_ASSERT(metadata != NULL);

            if (metadata->isInDirectory(directoryAbsolutePath)) {
                directoryItems.append(index);
                metadata->setIsSelected(!metadata->getIsSelected());
            }
        }

        emit allItemsSelectedChanged();
        artItemsModel->updateItems(directoryItems, QVector<int>() << ArtItemsModel::IsSelectedRole);
    }

    void FilteredArtItemsProxyModel::combineSelectedArtworks() {
        QVector<ArtItemInfo *> artworksList = getSelectedOriginalItemsWithIndices();
        m_CommandManager->combineArtworks(artworksList);
    }

    void FilteredArtItemsProxyModel::setSelectedItemsSaved() {
        QVector<int> indices = getSelectedOriginalIndices();
        ArtItemsModel *artItemsModel = getArtItemsModel();
        artItemsModel->setSelectedItemsSaved(indices);
    }

    void FilteredArtItemsProxyModel::removeSelectedArtworks() {
        QVector<int> indices = getSelectedOriginalIndices();
        ArtItemsModel *artItemsModel = getArtItemsModel();
        artItemsModel->removeSelectedArtworks(indices);
        updateFilter();
    }

    void FilteredArtItemsProxyModel::updateSelectedArtworks() {
        QVector<int> indices = getSelectedOriginalIndices();
        ArtItemsModel *artItemsModel = getArtItemsModel();
        artItemsModel->updateSelectedArtworks(indices);
    }

    void FilteredArtItemsProxyModel::saveSelectedArtworks() {
        // former patchSelectedArtworks
        QVector<int> indices = getSelectedOriginalIndices();
        ArtItemsModel *artItemsModel = getArtItemsModel();
        artItemsModel->saveSelectedArtworks(indices);
    }

    void FilteredArtItemsProxyModel::setSelectedForUpload() {
        QVector<ArtworkMetadata *> selectedArtworks = getSelectedOriginalItems();
        m_CommandManager->setArtworksForUpload(selectedArtworks);

        QVector<ArtItemInfo *> selectedArtworksWithIndices = getSelectedOriginalItemsWithIndices();
        emit needCheckItemsForWarnings(selectedArtworksWithIndices);
    }

    void FilteredArtItemsProxyModel::setSelectedForZipping() {
        QVector<ArtworkMetadata *> selectedArtworks = getSelectedOriginalItems();
        m_CommandManager->setArtworksForZipping(selectedArtworks);
    }

    bool FilteredArtItemsProxyModel::areSelectedArtworksSaved() {
        int modifiedSelectedCount = getModifiedSelectedCount();
        return modifiedSelectedCount == 0;
    }

    void FilteredArtItemsProxyModel::spellCheckSelected() {
        QVector<ArtworkMetadata *> selectedArtworks = getSelectedOriginalItems();
        m_CommandManager->submitForSpellCheck(selectedArtworks);
    }

    int FilteredArtItemsProxyModel::getModifiedSelectedCount() const {
        QVector<ArtworkMetadata *> selectedArtworks = getSelectedOriginalItems();
        int modifiedCount = 0;

        foreach (const ArtworkMetadata *metadata, selectedArtworks) {
            if (metadata->isModified()) {
                modifiedCount++;
            }
        }

        return modifiedCount;
    }

    void FilteredArtItemsProxyModel::removeArtworksDirectory(int index) {
        ArtItemsModel *artItemsModel = getArtItemsModel();
        artItemsModel->removeArtworksDirectory(index);
        emit selectedArtworksCountChanged();
    }

    void FilteredArtItemsProxyModel::checkForWarnings() {
        QVector<ArtItemInfo *> selectedArtworks = getSelectedOriginalItemsWithIndices();

        if (selectedArtworks.isEmpty()) {
            selectedArtworks = getAllItemsWithIndices();
        }

        emit needCheckItemsForWarnings(selectedArtworks);
    }

    void FilteredArtItemsProxyModel::reimportMetadataForSelected() {
        QVector<ArtworkMetadata *> selectedArtworks = getSelectedOriginalItems();
        m_CommandManager->setArtworksForIPTCProcessing(selectedArtworks);
        ArtItemsModel *artItemsModel = getArtItemsModel();
        artItemsModel->raiseArtworksAdded(selectedArtworks.count());
    }

    int FilteredArtItemsProxyModel::findSelectedItemIndex() const {
        int index = -1;
        QVector<int> indices = getSelectedOriginalIndices();
        if (indices.length() == 1) {
            index = indices.first();
        }

        return index;
    }

    void FilteredArtItemsProxyModel::removeMetadataInSelected() const {
        QVector<ArtItemInfo *> selectedArtworks = getSelectedOriginalItemsWithIndices();
        int flags = 0;
        Common::SetFlag(flags, Common::EditDesctiption);
        Common::SetFlag(flags, Common::EditKeywords);
        Common::SetFlag(flags, Common::EditTitle);
        Common::SetFlag(flags, Common::Clear);
        removeMetadataInItems(selectedArtworks, flags);
    }

    void FilteredArtItemsProxyModel::clearKeywords(int index) {
        ArtItemsModel *artItemsModel = getArtItemsModel();
        int originalIndex = getOriginalIndex(index);
        ArtworkMetadata *metadata = artItemsModel->getArtwork(originalIndex);
        Q_ASSERT(metadata != NULL);
        ArtItemInfo *info = new ArtItemInfo(metadata, originalIndex);
        removeKeywordsInItem(info);
    }

    void FilteredArtItemsProxyModel::focusNextItem(int index) {
        if (0 <= index && index < rowCount() - 1) {
            QModelIndex nextQIndex = this->index(index + 1, 0);
            QModelIndex sourceIndex = mapToSource(nextQIndex);
            ArtItemsModel *artItemsModel = getArtItemsModel();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(sourceIndex.row());

            if (metadata != NULL) {
                metadata->requestFocus(+1);
            }
        }
    }

    void FilteredArtItemsProxyModel::focusPreviousItem(int index) {
        if (0 < index && index < rowCount()) {
            QModelIndex nextQIndex = this->index(index - 1, 0);
            QModelIndex sourceIndex = mapToSource(nextQIndex);
            ArtItemsModel *artItemsModel = getArtItemsModel();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(sourceIndex.row());

            if (metadata != NULL) {
                metadata->requestFocus(-1);
            }
        }
    }

    void FilteredArtItemsProxyModel::itemSelectedChanged(bool value) {
        int plus = value ? +1 : -1;
        m_SelectedArtworksCount += plus;
        emit selectedArtworksCountChanged();
    }

    void FilteredArtItemsProxyModel::onSelectedArtworksRemoved() {
        m_SelectedArtworksCount--;
        emit selectedArtworksCountChanged();
    }

    void FilteredArtItemsProxyModel::removeMetadataInItems(const QVector<ArtItemInfo *> &itemsToClear, int flags) const {
        Commands::CombinedEditCommand *combinedEditCommand = new Commands::CombinedEditCommand(
                    flags,
                    itemsToClear);

        Commands::CommandResult *result = m_CommandManager->processCommand(combinedEditCommand);
        Commands::CombinedEditCommandResult *combinedResult = static_cast<Commands::CombinedEditCommandResult*>(result);
        m_CommandManager->updateArtworks(combinedResult->m_IndicesToUpdate);

        delete combinedResult;
        qDeleteAll(itemsToClear);
    }

    void FilteredArtItemsProxyModel::removeKeywordsInItem(ArtItemInfo *itemToClear) {
        int flags = 0;
        Common::SetFlag(flags, Common::EditKeywords);
        Common::SetFlag(flags, Common::Clear);
        removeMetadataInItems(QVector<ArtItemInfo *>() << itemToClear, flags);
    }

    void FilteredArtItemsProxyModel::setFilteredItemsSelected(bool selected) {
        ArtItemsModel *artItemsModel = getArtItemsModel();
        QVector<int> indices;
        int size = this->rowCount();
        indices.reserve(size);

        for (int row = 0; row < size; ++row) {
            QModelIndex proxyIndex = this->index(row, 0);
            QModelIndex originalIndex = this->mapToSource(proxyIndex);

            int index = originalIndex.row();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(index);
            Q_ASSERT(metadata != NULL);
            metadata->setIsSelected(selected);
            indices << index;
        }

        artItemsModel->updateItems(indices, QVector<int>() << ArtItemsModel::IsSelectedRole);
        emit allItemsSelectedChanged();
    }

    QVector<ArtworkMetadata *> FilteredArtItemsProxyModel::getSelectedOriginalItems() const {
        ArtItemsModel *artItemsModel = getArtItemsModel();
        QVector<ArtworkMetadata *> selectedArtworks;
        int size = this->rowCount();
        selectedArtworks.reserve(size);

        for (int row = 0; row < size; ++row) {
            QModelIndex proxyIndex = this->index(row, 0);
            QModelIndex originalIndex = this->mapToSource(proxyIndex);

            int index = originalIndex.row();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(index);

            if (metadata != NULL && metadata->getIsSelected()) {
                selectedArtworks.append(metadata);
            }
        }

        return selectedArtworks;
    }

    QVector<ArtItemInfo *> FilteredArtItemsProxyModel::getSelectedOriginalItemsWithIndices() const {
        ArtItemsModel *artItemsModel = getArtItemsModel();
        QVector<ArtItemInfo *> selectedArtworks;
        int size = this->rowCount();
        selectedArtworks.reserve(size);

        for (int row = 0; row < size; ++row) {
            QModelIndex proxyIndex = this->index(row, 0);
            QModelIndex originalIndex = this->mapToSource(proxyIndex);

            int index = originalIndex.row();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(index);

            if (metadata != NULL && metadata->getIsSelected()) {
                ArtItemInfo *info = new ArtItemInfo(metadata, index);
                selectedArtworks.append(info);
            }
        }

        return selectedArtworks;
    }

    QVector<ArtItemInfo *> FilteredArtItemsProxyModel::getAllItemsWithIndices() const {
        ArtItemsModel *artItemsModel = getArtItemsModel();
        QVector<ArtItemInfo *> selectedArtworks;
        int size = this->rowCount();
        selectedArtworks.reserve(size);

        for (int row = 0; row < size; ++row) {
            QModelIndex proxyIndex = this->index(row, 0);
            QModelIndex originalIndex = this->mapToSource(proxyIndex);

            int index = originalIndex.row();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(index);

            if (metadata != NULL) {
                ArtItemInfo *info = new ArtItemInfo(metadata, index);
                selectedArtworks.append(info);
            }
        }

        return selectedArtworks;
    }

    QVector<int> FilteredArtItemsProxyModel::getSelectedOriginalIndices() const {
        ArtItemsModel *artItemsModel = getArtItemsModel();
        QVector<int> selectedIndices;
        int size = this->rowCount();
        selectedIndices.reserve(size);

        for (int row = 0; row < size; ++row) {
            QModelIndex proxyIndex = this->index(row, 0);
            QModelIndex originalIndex = this->mapToSource(proxyIndex);

            int index = originalIndex.row();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(index);

            if (metadata != NULL && metadata->getIsSelected()) {
                selectedIndices.append(index);
            }
        }

        return selectedIndices;
    }

    void FilteredArtItemsProxyModel::forceUnselectAllItems() {
        ArtItemsModel *artItemsModel = getArtItemsModel();
        artItemsModel->forceUnselectAllItems();
        m_SelectedArtworksCount = 0;
        emit selectedArtworksCountChanged();
        emit allItemsSelectedChanged();
    }

    ArtItemsModel *FilteredArtItemsProxyModel::getArtItemsModel() const {
        QAbstractItemModel *sourceItemModel = sourceModel();
        ArtItemsModel *artItemsModel = dynamic_cast<ArtItemsModel *>(sourceItemModel);
        return artItemsModel;
    }

    bool FilteredArtItemsProxyModel::fitsSpecialKeywords(const QString &searchTerm, const ArtworkMetadata *metadata) const {
        bool hasMatch = false;

        if (searchTerm == QLatin1String("x:modified")) {
            hasMatch = metadata->isModified();
        } else if (searchTerm == QLatin1String("x:empty")) {
            hasMatch = metadata->isEmpty();
        } else if (searchTerm == QLatin1String("x:selected")) {
            hasMatch = metadata->getIsSelected();
        }

        return hasMatch;
    }

    bool FilteredArtItemsProxyModel::containsPartsSearch(ArtworkMetadata *metadata) const {
        bool hasMatch = false;
        Models::SettingsModel *settings = m_CommandManager->getSettingsModel();

        if (settings->getSearchUsingAnd()) {
            hasMatch = containsAllPartsSearch(metadata);
        } else {
            hasMatch = containsAnyPartsSearch(metadata);
        }

        return hasMatch;
    }

    bool FilteredArtItemsProxyModel::containsAnyPartsSearch(ArtworkMetadata *metadata) const {
        bool hasMatch = false;
        QStringList searchTerms = m_SearchTerm.split(QChar::Space, QString::SkipEmptyParts);

        const QString &description = metadata->getDescription();
        const QString &title = metadata->getTitle();
        const QString &filepath = metadata->getFilepath();

        int length = searchTerms.length();

        for (int i = 0; i < length; ++i) {
            const QString &searchTerm = searchTerms.at(i);

            hasMatch = fitsSpecialKeywords(searchTerm, metadata);

            if (!hasMatch) {
                hasMatch = description.contains(searchTerm, Qt::CaseInsensitive);
            }

            if (!hasMatch) {
                hasMatch = title.contains(searchTerm, Qt::CaseInsensitive);
            }

            if (!hasMatch) {
                hasMatch = filepath.contains(searchTerm, Qt::CaseInsensitive);
            }

            if (hasMatch) { break; }
        }

        if (!hasMatch) {
            for (int i = 0; i < length; ++i) {
                QString searchTerm = searchTerms[i];
                bool strictMatch = false;

                if ((searchTerm.length() > 0) && searchTerm[0] == QChar('!')) {
                    strictMatch = true;
                    searchTerm.remove(0, 1);
                }

                hasMatch = metadata->containsKeyword(searchTerm, strictMatch);
                if (hasMatch) { break; }
            }
        }

        return hasMatch;
    }

    bool FilteredArtItemsProxyModel::containsAllPartsSearch(ArtworkMetadata *metadata) const {
        bool hasMatch = false;
        QStringList searchTerms = m_SearchTerm.split(QChar::Space, QString::SkipEmptyParts);

        const QString &description = metadata->getDescription();
        const QString &title = metadata->getTitle();
        const QString &filepath = metadata->getFilepath();

        bool anyError = false;
        int length = searchTerms.length();

        for (int i = 0; i < length; ++i) {
            QString searchTerm = searchTerms[i];
            bool anyContains = false;
            bool strictMatch = false;

            anyContains = fitsSpecialKeywords(searchTerm, metadata);

            if (!anyContains) {
                anyContains = description.contains(searchTerm, Qt::CaseInsensitive);
            }

            if (!anyContains) {
                anyContains = title.contains(searchTerm, Qt::CaseInsensitive);
            }

            if (!anyContains) {
                anyContains = filepath.contains(searchTerm, Qt::CaseInsensitive);
            }

            if (!anyContains) {
                if ((searchTerm.length() > 0) && searchTerm[0] == QChar('!')) {
                    strictMatch = true;
                    searchTerm.remove(0, 1);
                }

                anyContains = metadata->containsKeyword(searchTerm, strictMatch);
            }

            if (!anyContains) {
                anyError = true;
                break;
            }
        }

        hasMatch = !anyError;
        return hasMatch;
    }

    bool FilteredArtItemsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        Q_UNUSED(sourceParent);

        if (m_SearchTerm.trimmed().isEmpty()) { return true; }

        ArtItemsModel *artItemsModel = getArtItemsModel();
        ArtworkMetadata *metadata = artItemsModel->getArtwork(sourceRow);

        bool hasMatch = containsPartsSearch(metadata);
        return hasMatch;
    }
}