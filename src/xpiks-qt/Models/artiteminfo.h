/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014 Taras Kushnir <kushnirTV@gmail.com>
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

#ifndef ARTITEMINFO_H
#define ARTITEMINFO_H

#include <QString>
#include <QSet>
#include "artworkmetadata.h"

namespace Models {
    class ArtItemInfo
    {
    public:
        ArtItemInfo(ArtworkMetadata *metadata) :
            m_ArtworkMetadata(metadata),
            m_IsSelected(false)
        {}
        ~ArtItemInfo() {}

    public:        
        bool isSelected() const { return m_IsSelected; }
        void select() { m_IsSelected = true; }
        void deselect() { m_IsSelected = false; }

    public:
        ArtworkMetadata *getOrigin() { return m_ArtworkMetadata; }

    private:
        ArtworkMetadata *m_ArtworkMetadata;
        bool m_IsSelected;
    };
}

#endif // ARTITEMINFO_H
