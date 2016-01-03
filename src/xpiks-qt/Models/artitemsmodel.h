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

#ifndef ARTSITEMSMODEL_H
#define ARTSITEMSMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QList>
#include <QVector>
#include <QPair>
#include <QUrl>
#include <QSize>
#include <QQuickTextDocument>
#include <QVector>
#include "abstractlistmodel.h"
#include "../Common/baseentity.h"

namespace Models {
    class ArtworkMetadata;
    class ArtItemInfo;

    class ArtItemsModel :
            public AbstractListModel,
            public Common::BaseEntity
    {
        Q_OBJECT
        Q_PROPERTY(int modifiedArtworksCount READ getModifiedArtworksCount NOTIFY modifiedArtworksCountChanged)
    public:
        ArtItemsModel(QObject *parent = 0) :
            AbstractListModel(parent),
            Common::BaseEntity()
        {}

        virtual ~ArtItemsModel();

    public:
        enum ArtItemsRoles {
            ArtworkDescriptionRole = Qt::UserRole + 1,
            EditArtworkDescriptionRole,
            ArtworkFilenameRole,
            ArtworkTitleRole,
            EditArtworkTitleRole,
            KeywordsStringRole,
            KeywordsCountRole,
            IsModifiedRole,
            IsSelectedRole,
            EditIsSelectedRole
        };

    public:
        virtual ArtworkMetadata *createMetadata(const QString &filepath) const;

    public:
        int getModifiedArtworksCount();
        void updateModifiedCount() { emit modifiedArtworksCountChanged(); }
        void updateItems(const QVector<int> &indices, const QVector<int> &roles);
        void forceUnselectAllItems() const;

    public:
        Q_INVOKABLE void updateLastN(int N);
        /*Q_INVOKABLE*/ void removeArtworksDirectory(int index);
        Q_INVOKABLE void removeKeywordAt(int metadataIndex, int keywordIndex);
        Q_INVOKABLE void removeLastKeyword(int metadataIndex);
        Q_INVOKABLE void appendKeyword(int metadataIndex, const QString &keyword);
        Q_INVOKABLE void pasteKeywords(int metadataIndex, const QStringList &keywords);
        Q_INVOKABLE void suggestCorrections(int metadataIndex);
        Q_INVOKABLE void clearKeywords(int metadataIndex);
        Q_INVOKABLE void backupItem(int metadataIndex);
        Q_INVOKABLE void combineArtwork(int index) { doCombineArtwork(index); }
        Q_INVOKABLE int dropFiles(const QList<QUrl> &urls);

        /*Q_INVOKABLE*/ void setSelectedItemsSaved(const QVector<int> &selectedIndices);
        /*Q_INVOKABLE*/ void removeSelectedArtworks(QVector<int> &selectedIndices);
        /*Q_INVOKABLE*/ void updateSelectedArtworks(const QVector<int> &selectedIndices);
        /*Q_INVOKABLE*/ void saveSelectedArtworks(const QVector<int> &selectedIndices);

        Q_INVOKABLE QObject *getArtworkItself(int index) const;
        Q_INVOKABLE QSize retrieveImageSize(int metadataIndex) const;
        Q_INVOKABLE QString retrieveFileSize(int metadataIndex) const;
        Q_INVOKABLE QString getArtworkFilepath(int index) const;

        Q_INVOKABLE int addRecentDirectory(const QString &directory);
        Q_INVOKABLE void initDescriptionHighlighting(int metadataIndex, QQuickTextDocument *document);
        Q_INVOKABLE void initTitleHighlighting(int metadataIndex, QQuickTextDocument *document);

        Q_INVOKABLE void editKeyword(int metadataIndex, int keywordIndex, const QString &replacement);
        Q_INVOKABLE void plainTextEdit(int metadataIndex, const QString &rawKeywords);

    public:
        virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
        virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const;
        virtual bool setData(const QModelIndex &index, const QVariant & value, int role = Qt::EditRole);

    public slots:
        int addLocalArtworks(const QList<QUrl> &artworksPaths);
        int addLocalDirectories(const QList<QUrl> &directories);
        void itemModifiedChanged(bool) { updateModifiedCount(); }

    public:
        void beginAccountingFiles(int filesCount);
        void beginAccountingFiles(int start, int end);
        void endAccountingFiles();

    public:
        void insertArtwork(int index, ArtworkMetadata *metadata);
        void appendMetadata(ArtworkMetadata *metadata);
        void removeArtworks(const QVector<QPair<int, int> > &ranges) { doRemoveItemsInRanges(ranges); }
        ArtworkMetadata *getArtwork(int index) const;
        void raiseArtworksAdded(int count) { emit artworksAdded(count); }
        void updateItemsAtIndices(const QVector<int> &indices);
        void setAllItemsSelected(bool selected);

    private:
        void updateItemAtIndex(int metadataIndex);
        int addDirectories(const QStringList &directories);
        void doAddDirectory(const QString &directory, QStringList &filesList);
        int addFiles(const QStringList &filepath);
        void getSelectedArtworks(QVector<ArtworkMetadata *> &selectedArtworks) const;

    private:
        void doCombineArtwork(int index);

    signals:
        void needCheckItemsForWarnings(const QVector<ArtItemInfo*> &artworks);
        void modifiedArtworksCountChanged();
        void artworksChanged();
        void artworksAdded(int count);
        void selectedArtworkRemoved();

    protected:
       virtual QHash<int, QByteArray> roleNames() const;

    protected:
        void removeInnerItem(int row);

    private:
        void doRemoveItemsAtIndices(QVector<int> &indicesToRemove);
        void doRemoveItemsInRanges(const QVector<QPair<int, int> > &rangesToRemove);
        void getSelectedItemsIndices(QVector<int> &indices);

    private:
        void fillStandardRoles(QVector<int> &roles) const;

    private:
        QVector<ArtworkMetadata*> m_ArtworkList;
    };
}

#endif // ARTSITEMSMODEL_H