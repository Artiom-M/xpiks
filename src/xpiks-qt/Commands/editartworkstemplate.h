/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef EDITARTWORKSTEMPLATE_H
#define EDITARTWORKSTEMPLATE_H

#include "iartworkscommandtemplate.h"
#include <QString>
#include "../Common/flags.h"
#include "../UndoRedo/artworkmetadatabackup.h"

namespace Artworks {
    class ArtworkMetadata;
}

namespace Commands {
    class EditArtworksTemplate: public IArtworksCommandTemplate
    {
    public:
        EditArtworksTemplate(const QString &title,
                             const QString &description,
                             const QString &keywords,
                             Common::CombinedEditFlags editFlags);

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(Artworks::ArtworksSnapshot &snapshot) override;
        virtual void undo(Artworks::ArtworksSnapshot &snapshot) override;

    private:
        void setKeywords(Artworks::ArtworkMetadata *artwork) const;
        void setDescription(Artworks::ArtworkMetadata *artwork) const;
        void setTitle(Artworks::ArtworkMetadata *artwork) const;

    private:
        QString m_Description;
        QString m_Title;
        QStringList m_Keywords;
        Common::CombinedEditFlags m_EditFlags;
        std::vector<UndoRedo::ArtworkMetadataBackup> m_ArtworksBackups;
    };
}

#endif // EDITARTWORKSTEMPLATE_H
