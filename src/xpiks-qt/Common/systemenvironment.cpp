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

#include <QChar>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <Qt>
#include <QtDebug>
#include <QtGlobal>

#include "Common/defines.h"  // IWYU pragma: keep
#include "Common/logging.h"
#include "Helpers/constants.h"
#include "Helpers/filehelpers.h"

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
#include <QDateTime>
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    #define XPIKS_DATA_LOCATION_TYPE QStandardPaths::AppDataLocation
    #if defined(UI_TESTS)
        #define XPIKS_USERDATA_PATH (QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "_ui-tests/" + STRINGIZE(BRANCHNAME))
    #elif defined(INTEGRATION_TESTS)
        #define XPIKS_USERDATA_PATH (QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "_integration-tests/" + STRINGIZE(BRANCHNAME))
    #elif defined(QT_DEBUG)
        #define XPIKS_USERDATA_PATH (QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "_debug/" + STRINGIZE(BRANCHNAME))
    #else
        #define XPIKS_USERDATA_PATH QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
    #endif
#else
    #define XPIKS_DATA_LOCATION_TYPE QStandardPaths::DataLocation
    #define XPIKS_USERDATA_PATH (QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + STRINGIZE(BRANCHNAME))
#endif

namespace Common {
    SystemEnvironment::SystemEnvironment(const QStringList &appArguments) {
        m_Root = XPIKS_USERDATA_PATH;

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
        m_SessionTag = "session-" + QDateTime::currentDateTimeUtc().toString("ddMMyyyy-hhmmss");
        m_Root += "/" + m_SessionTag;
#endif

        bool portable = false;

#ifdef Q_OS_WIN
        portable = appArguments.contains("--portable", Qt::CaseInsensitive);
#else
        Q_UNUSED(appArguments);
#endif

        if (portable || m_Root.isEmpty()) {
            m_Root = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/" + "settings");
        }

        m_InMemoryExperiment = appArguments.contains("--in-memory", Qt::CaseInsensitive);
        m_IsRecoveryMode = appArguments.contains("--recovery", Qt::CaseInsensitive);

        LOG_INFO << "Configs root is" << m_Root;
        LOG_INFO << "Extra files search locations:" << QStandardPaths::standardLocations(XPIKS_DATA_LOCATION_TYPE);
        LOG_INFO << "In-memory experiment:" << m_InMemoryExperiment;
        LOG_INFO << "Recovery mode:" << m_IsRecoveryMode;
    }

    void SystemEnvironment::ensureSystemDirectoriesExist() {
        if (m_InMemoryExperiment) { return; }
        ensureDirExists(Constants::LOGS_DIR);
        ensureDirExists(Constants::STATES_DIR);
        ensureDirExists(Constants::CRASHES_DIR);
    }

    QString SystemEnvironment::path(const QStringList &path) {
        QString result = QDir::cleanPath(m_Root + QChar('/') + path.join(QChar('/')));
        return result;
    }

    bool SystemEnvironment::ensureDirExists(const QString &name) {
        QString dirpath = path(QStringList() << name);
        bool result = Helpers::ensureDirectoryExists(dirpath);
        return result;
    }
}
