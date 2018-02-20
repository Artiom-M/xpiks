/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "systemenvironment.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include "defines.h"
#include "../Helpers/filehelpers.h"
#include "../Helpers/constants.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    #define XPIKS_DATA_LOCATION_TYPE QStandardPaths::AppDataLocation
    #ifdef QT_DEBUG
        #define XPIKS_USERDATA_PATH (QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + STRINGIZE(BRANCHNAME))
    #else
        #define XPIKS_USERDATA_PATH QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
    #endif
#else
    #define XPIKS_DATA_LOCATION_TYPE QStandardPaths::DataLocation
    #define XPIKS_USERDATA_PATH (QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + STRINGIZE(BRANCHNAME))
#endif

namespace Common {
    SystemEnvironment::SystemEnvironment(const QStringList &appArguments) {
        Q_UNUSED(appArguments);
        m_Root = XPIKS_USERDATA_PATH;
        bool portable = false;
#ifdef Q_OS_WIN
        portable = appArguments.contains("--portable", Qt::CaseInsensitive);
#endif

        if (portable || m_Root.isEmpty()) {
            m_Root = QCoreApplication::applicationDirPath();
        }

        LOG_DEBUG << "Configs root is" << m_Root;
        LOG_DEBUG << "Extra files search locations:" << QStandardPaths::standardLocations(XPIKS_DATA_LOCATION_TYPE);
    }

    void SystemEnvironment::ensureSystemDirectoriesExist() {
        ensureDirExists(Constants::LOGS_DIR);
        ensureDirExists(Constants::STATES_DIR);
    }

    QString SystemEnvironment::fileInDir(const QString &filename, const QString &dirname) {
        const QString dirpath = QDir::cleanPath(m_Root + QDir::separator() + dirname);
        QDir dir(dirpath);
#ifndef CORE_TESTS
        Q_ASSERT(dir.exists());
#endif
        QString filepath = dir.filePath(filename);
        return filepath;
    }

    QString SystemEnvironment::filepath(const QString &name) {
        QString path = QDir::cleanPath(m_Root + QDir::separator() + name);
        return path;
    }

    QString SystemEnvironment::dirpath(const QString &name) {
        QString path = QDir::cleanPath(m_Root + QDir::separator() + name);
        return path;
    }

    void SystemEnvironment::ensureDirExists(const QString &name) {
        QString path = dirpath(name);
        Helpers::ensureDirectoryExists(path);
    }
}
