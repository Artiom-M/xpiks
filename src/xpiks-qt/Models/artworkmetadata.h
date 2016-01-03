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

#ifndef IMAGEMETADATA_H
#define IMAGEMETADATA_H

#include <QAbstractListModel>
#include <QReadWriteLock>
#include <QStringList>
#include <QFileInfo>
#include <QString>
#include <QVector>
#include <QSet>
#include <QSize>
#include "../Common/basickeywordsmodel.h"

class QTextDocument;

namespace Models {
    class SettingsModel;

    class ArtworkMetadata : public Common::BasicKeywordsModel {
        Q_OBJECT
    public:
        ArtworkMetadata(const QString &filepath);
        virtual ~ArtworkMetadata();

    public:
        bool initialize(const QString &title,
                        const QString &description, const QString &rawKeywords, bool overwrite = true);

    public:
        const QString &getFilepath() const { return m_ArtworkFilepath; }
        virtual QString getDirectory() const { QFileInfo fi(m_ArtworkFilepath); return fi.absolutePath(); }

    public:
        bool isInDirectory(const QString &directoryAbsolutePath) const;
        bool isModified() const { return m_IsModified; }
        bool getIsSelected() const { return m_IsSelected; }
        bool isInitialized() const { return m_IsInitialized; }
        QSize getSize() const { return m_ImageSize; }

    public:
        virtual void clearModel();
        virtual bool editKeyword(int index, const QString &replacement);

    public:
        virtual bool setDescription(const QString &value) {
            bool result = BasicKeywordsModel::setDescription(value);
            if (result) { markModified(); }
            return result;
        }

        bool setTitle(const QString &value) {
            bool result = BasicKeywordsModel::setTitle(value);
            if (result) { markModified(); }
            return result;
        }

        bool setIsSelected(bool value) {
            bool result = m_IsSelected != value;
            if (result) {
                m_IsSelected = value;
                selectedChanged(value);
                fileSelectedChanged(m_ArtworkFilepath, value);
            }
            return result;
        }

        void setSize(const QSize &size) { m_ImageSize = size; }

        void resetSelected() {
            if (m_IsSelected) {
                m_IsSelected = false;
                fileSelectedChanged(m_ArtworkFilepath, false);
            }
        }

    public:
        bool removeKeywordAt(int index);
        bool removeLastKeyword();
        virtual bool appendKeyword(const QString &keyword);
        virtual int appendKeywords(const QStringList &keywordsList);

    public:
        void markModified() { if (!m_IsModified) { m_IsModified = true; emit modifiedChanged(m_IsModified); } }
        void setModified() { m_IsModified = true; }
        void resetModified() { m_IsModified = false; }
        void requestFocus(int directionSign) { emit focusRequested(directionSign); }

    signals:
         void modifiedChanged(bool newValue);
         void selectedChanged(bool newValue);
         void fileSelectedChanged(const QString &filepath, bool newValue);
         void focusRequested(int directionSign);

    private:
         QSize m_ImageSize;
         QString m_ArtworkFilepath;
         volatile bool m_IsModified;
         volatile bool m_IsSelected;
         volatile bool m_IsInitialized;
    };
}

Q_DECLARE_METATYPE(Models::ArtworkMetadata*)

#endif // IMAGEMETADATA_H