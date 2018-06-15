/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "filesdirectoriescollection.h"
#include "filescollection.h"
#include "directoriescollection.h"
#include <QDir>

namespace Filesystem {
    FilesDirectoriesCollection::FilesDirectoriesCollection(const QList<QUrl> &urls) {
        sortFilesDirectories(urls);
    }

    void FilesDirectoriesCollection::sortFilesDirectories(const QList<QUrl> &urls) {
        QList<QUrl> directories, files;
        directories.reserve(urls.count()/2);
        files.reserve(urls.count());

        foreach(const QUrl &url, urls) {
            Q_ASSERT(url.isLocalFile());
            const bool isDirectory = QDir(url.toLocalFile()).exists();

            if (isDirectory) {
                directories.append(url);
            } else {
                files.append(url);
            }
        }

        FilesCollection filesCollection(files);
        DirectoriesCollection dirsCollection(directories);

        m_Images.append(filesCollection.getImages());
        m_Images.append(dirsCollection.getImages());

        m_Vectors.append(filesCollection.getVectors());
        m_Vectors.append(dirsCollection.getVectors());

        m_Videos.append(filesCollection.getVideos());
        m_Videos.append(dirsCollection.getVideos());
    }
}
