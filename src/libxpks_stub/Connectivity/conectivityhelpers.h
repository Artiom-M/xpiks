/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONECTIVITYHELPERS_H
#define CONECTIVITYHELPERS_H

#include <memory>
#include <vector>

class QStringList;

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Encryption {
    class SecretsManager;
}

namespace Models {
    class SettingsModel;
    class UploadInfo;
}

namespace libxpks {
    namespace net {
        class UploadBatch;
        class UploadContext;

        void extractFilePathes(const Artworks::ArtworksSnapshot &artworksSnapshot,
                               QStringList &filePathes,
                               QStringList &zipsPathes);

        void generateUploadContexts(const std::vector<std::shared_ptr<Models::UploadInfo> > &uploadInfos,
                                    std::vector<std::shared_ptr<UploadContext> > &contexts,
                                    Encryption::SecretsManager &secretsManager,
                                    Models::SettingsModel &settingsModel);

        std::vector<std::shared_ptr<UploadBatch> > generateUploadBatches(const Artworks::ArtworksSnapshot &artworksToUpload,
                                                                         const std::vector<std::shared_ptr<Models::UploadInfo> > &uploadInfos,
                                                                         Encryption::SecretsManager &secretsManager,
                                                                         Models::SettingsModel &settingsModel);
    }
}

#endif // CONECTIVITYHELPERS_H
