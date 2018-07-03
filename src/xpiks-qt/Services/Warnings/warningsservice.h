/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef WARNINGSSERVICE_H
#define WARNINGSSERVICE_H

#include <QObject>
#include <Artworks/artworkmetadata.h>
#include <Common/flags.h>
#include "warningssettingsmodel.h"
#include <Artworks/artworkssnapshot.h>
#include <Common/isystemenvironment.h>

namespace Warnings {
    class WarningsCheckingWorker;

    class WarningsService: public QObject
    {
        Q_OBJECT

    public:
        explicit WarningsService(Common::ISystemEnvironment &environment, QObject *parent=0);
        virtual ~WarningsService() {}

    public:
        void initWarningsSettings();
        const WarningsSettingsModel *getWarningsSettingsModel() const { return &m_WarningsSettingsModel; }
        void cancelCurrentBatch();

    public:
        virtual void startService(const std::shared_ptr<Services::ServiceStartParams> &params);
        virtual void stopService();

        virtual bool isAvailable() const { return true; }
        virtual bool isBusy() const;

        virtual void submitItem(Artworks::ArtworkMetadata *item);
        virtual void submitItem(Artworks::ArtworkMetadata *item, Common::WarningsCheckFlags flags);
        virtual void submitItems(const Artworks::WeakArtworksSnapshot &items);

    private slots:
        void workerDestoyed(QObject *object);
        void workerStopped();
        void updateWarningsSettings();

    signals:
        void queueIsEmpty();

    private:
        WarningsCheckingWorker *m_WarningsWorker;
        WarningsSettingsModel m_WarningsSettingsModel;
        bool m_IsStopped;
    };
}

#endif // WARNINGSSERVICE_H
