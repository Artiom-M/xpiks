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

#include <QDebug>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>
#include <QString>
#include "suggestionqueryengine.h"
#include "suggestionartwork.h"
#include "keywordssuggestor.h"
#include "../Encryption/aes-qt.h"

namespace Suggestion {
    SuggestionQueryEngine::SuggestionQueryEngine(KeywordsSuggestor *keywordsSuggestor):
                    QObject(keywordsSuggestor),
                    m_NetworkManager(this),
                    m_Suggestor(keywordsSuggestor)
    {
        m_ClientId = Encryption::decodeText("28a2a9b917961a0cbc343c81b2dd0f6618377f9210aa3182e5cc9f5588f914d918ede1533c9e06b91769c89e80909743", "MasterPassword");
        m_ClientSecret = Encryption::decodeText("5092d9a967c2f19b57aac29bc09ac3b9e6ae5baec1a371331b73ff24f1625d95c4f3fef90bdacfbe9b0b3803b48c269192bc55f14bb9c2b5a16d650cd641b746eb384fcf9dbd53a96f1f81215921b04409f3635ecf846ffdf01ee04ba76624c9", "MasterPassword");

        QObject::connect(&m_NetworkManager, SIGNAL(finished(QNetworkReply*)),
                         this, SLOT(replyReceived(QNetworkReply*)));
    }

    void SuggestionQueryEngine::submitQuery(const QStringList &queryKeywords) {
        QUrl url = buildQuery(queryKeywords);
        QNetworkRequest request(url);

        QString authStr = QString("%1:%2").arg(m_ClientId).arg(m_ClientSecret);
        QString headerData = "Basic " + authStr.toLocal8Bit().toBase64();
        request.setRawHeader("Authorization", headerData.toLocal8Bit());

        m_NetworkManager.get(request);
    }

    void SuggestionQueryEngine::replyReceived(QNetworkReply *networkReply) {
        if (networkReply->error() == QNetworkReply::NoError) {
            QJsonDocument document = QJsonDocument::fromJson(networkReply->readAll());
            QJsonObject jsonObject = document.object();
            QJsonValue dataValue = jsonObject["data"];

            if (dataValue.isArray()) {
                QList<SuggestionArtwork*> suggestionArtworks;
                parseResponse(dataValue.toArray(), suggestionArtworks);
                m_Suggestor->setSuggestedArtworks(suggestionArtworks);
            }
        } else {
            qDebug() << networkReply->errorString();
        }

        m_Suggestor->unsetInProgress();
        networkReply->deleteLater();
    }

    void SuggestionQueryEngine::parseResponse(const QJsonArray &jsonArray, QList<SuggestionArtwork*> &suggestionArtworks) {
        foreach (const QJsonValue &value, jsonArray) {
            QJsonObject imageResult = value.toObject();

            if (imageResult.contains("assets")) {
                QJsonObject assetsObject = imageResult["assets"].toObject();
                if (assetsObject.contains("large_thumb")) {
                    QJsonObject thumb = assetsObject["large_thumb"].toObject();
                    if (thumb.contains("url")) {
                        QString url = thumb["url"].toString();
                        QStringList keywordsList;

                        if (imageResult["keywords"].isArray()) {
                            foreach (const QJsonValue &keyword, imageResult["keywords"].toArray()) {
                                keywordsList.append(keyword.toString());
                            }
                        }

                        SuggestionArtwork *artwork = new SuggestionArtwork(url, keywordsList);
                        suggestionArtworks.append(artwork);
                    }
                }
            }
        }
    }

    QUrl SuggestionQueryEngine::buildQuery(const QStringList &queryKeywords) const {
        QUrlQuery urlQuery;

        QString queryString = QString("https://api.shutterstock.com/v2/images/search");

        urlQuery.addQueryItem("language", "en");
        urlQuery.addQueryItem("view", "full");
        urlQuery.addQueryItem("per_page", "75");
        urlQuery.addQueryItem("query", queryKeywords.join(' '));

        QUrl url;
        url.setUrl(queryString);
        url.setQuery(urlQuery);
        return url;
    }
}

