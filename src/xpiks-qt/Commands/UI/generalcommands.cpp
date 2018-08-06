/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "generalcommands.h"
#include <Models/settingsmodel.h>
#include <Encryption/secretsmanager.h>
#include <Models/Artworks/artworkslistmodel.h>

namespace Commands {
    namespace UI {
        void SetMasterPasswordCommand::execute(const QJSValue &value) {
            LOG_DEBUG << "#";
            m_Target.onMasterPasswordSet(m_Source);
        }

        void RemoveUnavailableFilesCommand::execute(const QJSValue &) {
            LOG_DEBUG << "#";
            m_ArtworksListModel.purgeUnavailableFiles();
        }
    }
}
