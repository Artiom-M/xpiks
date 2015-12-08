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

#include <QTime>
#include <QSettings>
#include <QRegExp>
#include <QUrl>
#include <QUrlQuery>
#include <QSysInfo>
#include <QNetworkRequest>
#include <QByteArray>
#include <QDebug>
#include <QNetworkReply>
#include "telemetryservice.h"
#include "telemetryworker.h"
#include "analyticsuserevent.h"

namespace Conectivity {
    TelemetryService::TelemetryService(const QString &userId, QObject *parent) :
        QObject(parent),
        m_NetworkManager(this),
        m_UserAgentId(userId)
    {
        QObject::connect(&m_NetworkManager, SIGNAL(finished(QNetworkReply*)),
                         this, SLOT(replyReceived(QNetworkReply*)));

        m_ReportingEndpoint = "";
    }

    void TelemetryService::reportAction(UserAction action) {
        AnalyticsUserEvent userEvent(action);

        QUrlQuery query;
        query.addQueryItem(QLatin1String("idsite"), QLatin1String("1"));
        query.addQueryItem(QLatin1String("rec"), QLatin1String("1"));
        query.addQueryItem(QLatin1String("url"), QLatin1String("client"));
        query.addQueryItem(QLatin1String("action_name"), userEvent.getActionString());
        query.addQueryItem(QLatin1String("_id"), m_UserAgentId);
        query.addQueryItem(QLatin1String("rand"), QString::number(qrand()));
        query.addQueryItem(QLatin1String("apiv"), QLatin1String("1"));
        query.addQueryItem(QLatin1String("h"), QString::number(userEvent.getHour()));
        query.addQueryItem(QLatin1String("m"), QString::number(userEvent.getMinute()));
        query.addQueryItem(QLatin1String("s"), QString::number(userEvent.getSecond()));
        query.addQueryItem(QLatin1String("send_image"), QLatin1String("0"));
        /*query.addQueryItem(QLatin1String("_cvar"),
                           QString("{\"1\":[\"OS_type\",\"%1\"],\"2\":[\"OS_version\",\"%2\"]}")
                           .arg(QSysInfo::productType())
                           .arg(QSysInfo::productVersion()));
                           */

        QUrl reportingUrl;
        reportingUrl.setUrl(m_ReportingEndpoint);
        reportingUrl.setQuery(query);

#ifdef QT_DEBUG
        qDebug() << "Telemetry request" << reportingUrl;
#endif

        QNetworkRequest request(reportingUrl);
        //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        QNetworkReply *reply = m_NetworkManager.get(request);
        QObject::connect(this, SIGNAL(cancelAllQueries()),
                         reply, SLOT(abort()));
    }

    void TelemetryService::replyReceived(QNetworkReply *networkReply) {
        if (networkReply->error() == QNetworkReply::NoError) {
            qDebug() << "Telemetry report submited successfully";
        } else {
            // TODO: add tracking of failed items

            qDebug() << "Failed to process a telemetry report." << networkReply->errorString();;
        }

        networkReply->deleteLater();
    }
}

