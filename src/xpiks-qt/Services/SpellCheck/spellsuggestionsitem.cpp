/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spellsuggestionsitem.h"

#include <utility>

#include <QAbstractItemModel>
#include <QByteArray>
#include <QFlags>
#include <QVector>
#include <QtDebug>
#include <QtGlobal>

#include "Common/flags.h"
#include "Common/logging.h"
#include "Services/SpellCheck/ispellcheckable.h"

namespace SpellCheck {
    SpellSuggestionsItem::SpellSuggestionsItem(const QString &word,
                                               const QString &origin) :
        QAbstractListModel(),
        m_Word(word),
        m_ReplacementOrigin(origin),
        m_ReplacementIndex(-1),
        m_ReplacementSucceeded(false)
    {
    }

    SpellSuggestionsItem::SpellSuggestionsItem(const QString &word):
        QAbstractListModel(),
        m_Word(word),
        m_ReplacementOrigin(word),
        m_ReplacementIndex(-1),
        m_ReplacementSucceeded(false)
    {
    }

    bool SpellSuggestionsItem::setReplacementIndex(int value) {
        bool result = (value != m_ReplacementIndex) && (value != -1);

        if (result) {
            QVector<int> roles;
            roles << IsSelectedRole;
            int prevIndexToUpdate = m_ReplacementIndex;
            m_ReplacementIndex = value;

            if (prevIndexToUpdate != -1) {
                QModelIndex prev = this->index(prevIndexToUpdate);
                emit dataChanged(prev, prev, roles);
            }

            QModelIndex curr = this->index(value);
            emit dataChanged(curr, curr, roles);
        } else {
            if ((value == m_ReplacementIndex) ||
                    (value == -1 && m_ReplacementIndex != -1)) {
                int prevIndexToUpdate = m_ReplacementIndex;
                m_ReplacementIndex = -1;

                QModelIndex prev = this->index(prevIndexToUpdate);
                QVector<int> roles;
                roles << IsSelectedRole;
                emit dataChanged(prev, prev, roles);
                result = true;
            }
        }

        return result;
    }

    void SpellSuggestionsItem::setSuggestions(const QStringList &suggestions) {
        beginResetModel();
        m_Suggestions.clear();
        m_Suggestions.append(suggestions);
        endResetModel();
    }

    int SpellSuggestionsItem::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return m_Suggestions.length();
    }

    QVariant SpellSuggestionsItem::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if (row < 0 || row >= m_Suggestions.length()) { return QVariant(); }

        switch (role) {
        case SuggestionRole:
            return m_Suggestions.at(row);
        case IsSelectedRole:
            return (m_ReplacementIndex == row);
        default:
            return QVariant();
        }
    }

    Qt::ItemFlags SpellSuggestionsItem::flags(const QModelIndex &index) const {
        int row = index.row();
        if (row < 0 || row >= m_Suggestions.length()) {
            return Qt::ItemIsEnabled;
        }

        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    bool SpellSuggestionsItem::setData(const QModelIndex &index, const QVariant &value, int role) {
        int row = index.row();
        if (row < 0 || row >= m_Suggestions.length()) return false;

        bool result = false;

        switch (role) {
        case EditReplacementIndexRole:
            result = setReplacementIndex(value.toInt());
            break;
        default:
            return false;
        }

        return result;
    }

    QHash<int, QByteArray> SpellSuggestionsItem::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[SuggestionRole] = "suggestion";
        roles[IsSelectedRole] = "isselected";
        roles[EditReplacementIndexRole] = "editreplacementindex";
        return roles;
    }

    MetadataSpellSuggestionsItem::MetadataSpellSuggestionsItem(const QString &word,
                                                               const QString &origin,
                                                               ISpellCheckable *spellCheckable):
        SpellSuggestionsItem(word, origin),
        m_SpellCheckable(spellCheckable)
    {
        Q_ASSERT(spellCheckable != nullptr);
    }

    MetadataSpellSuggestionsItem::MetadataSpellSuggestionsItem(const QString &word,
                                                               ISpellCheckable *spellCheckable):
        SpellSuggestionsItem(word),
        m_SpellCheckable(spellCheckable)
    {
        Q_ASSERT(spellCheckable != nullptr);
    }

    KeywordSpellSuggestions::KeywordSpellSuggestions(const QString &keyword,
                                                     size_t originalIndex,
                                                     const QString &origin,
                                                     ISpellCheckable *spellCheckable) :
        MetadataSpellSuggestionsItem(keyword, origin, spellCheckable),
        m_OriginalIndex(originalIndex),
        m_ReplaceResult(Common::KeywordReplaceResult::Unknown)
    {
    }

    KeywordSpellSuggestions::KeywordSpellSuggestions(const QString &keyword,
                                                     size_t originalIndex,
                                                     ISpellCheckable *spellCheckable):
        MetadataSpellSuggestionsItem(keyword, spellCheckable),
        m_OriginalIndex(originalIndex),
        m_ReplaceResult(Common::KeywordReplaceResult::Unknown)
    {
    }

    void KeywordSpellSuggestions::replaceToSuggested() {
        if (anyReplacementSelected()) {
            const QString &word = getWord();
            const QString &replacement = getReplacement();
            this->replaceToSuggested(word, replacement);
        }
    }

    void KeywordSpellSuggestions::replaceToSuggested(const QString &word, const QString &replacement) {
        LOG_INFO << word << "-->" << replacement;
        auto *item = getSpellCheckable();
        Common::KeywordReplaceResult result = item->fixKeywordSpelling(m_OriginalIndex, word, replacement);
        setReplacementSucceeded(result == Common::KeywordReplaceResult::Succeeded);
        m_ReplaceResult = result;
    }

    DescriptionSpellSuggestions::DescriptionSpellSuggestions(const QString &word,
                                                             ISpellCheckable *spellCheckable):
        MetadataSpellSuggestionsItem(word, spellCheckable)
    {
    }

    void DescriptionSpellSuggestions::replaceToSuggested() {
        if (anyReplacementSelected()) {
            const QString &word = getWord();
            const QString &replacement = getReplacement();
            this->replaceToSuggested(word, replacement);
        }
    }

    void DescriptionSpellSuggestions::replaceToSuggested(const QString &word, const QString &replacement) {
        LOG_INFO << word << "-->" << replacement;
        auto *item = getSpellCheckable();
        bool success = item->fixDescriptionSpelling(word, replacement);
        setReplacementSucceeded(success);

        if (!success) {
            LOG_WARNING << "Failed to replace in description" << word << "to" << replacement;
        }
    }

    TitleSpellSuggestions::TitleSpellSuggestions(const QString &word,
                                                 ISpellCheckable *spellCheckable):
        MetadataSpellSuggestionsItem(word, spellCheckable)
    {
    }

    void TitleSpellSuggestions::replaceToSuggested() {
        if (anyReplacementSelected()) {
            const QString &word = getWord();
            const QString &replacement = getReplacement();
            this->replaceToSuggested(word, replacement);
        }
    }

    void TitleSpellSuggestions::replaceToSuggested(const QString &word, const QString &replacement) {
        LOG_INFO << word << "-->" << replacement;
        auto *item = getSpellCheckable();
        bool success = item->fixTitleSpelling(word, replacement);
        setReplacementSucceeded(success);

        if (!success) {
            LOG_WARNING << "Failed to replace in title" << word << "to" << replacement;
        }
    }

    CombinedSpellSuggestions::CombinedSpellSuggestions(const QString &word,
                                                       std::vector<std::shared_ptr<SpellSuggestionsItem> > &suggestions):
        SpellSuggestionsItem(word, tr("multireplace")),
        m_SpellSuggestions(std::move(suggestions))
    {
        Q_ASSERT(!m_SpellSuggestions.empty());
    }

    std::vector<std::shared_ptr<KeywordSpellSuggestions>> CombinedSpellSuggestions::getKeywordsDuplicateSuggestions() const {
        std::vector<std::shared_ptr<KeywordSpellSuggestions>> keywordsSuggestions;

        QStringList suggestions = QStringList() << this->getReplacement();
        for (auto &item: m_SpellSuggestions) {
            std::shared_ptr<KeywordSpellSuggestions> keywordsItem = std::dynamic_pointer_cast<KeywordSpellSuggestions>(item);
            if (keywordsItem && keywordsItem->isPotentialDuplicate()) {
                keywordsItem->setSuggestions(suggestions);
                keywordsItem->setReplacementIndex(0);
                keywordsSuggestions.push_back(keywordsItem);
            }
        }

        return keywordsSuggestions;
    }

    void CombinedSpellSuggestions::replaceToSuggested() {
        if (anyReplacementSelected()) {
            const QString &word = getWord();
            const QString &replacement = getReplacement();
            this->replaceToSuggested(word, replacement);
        }
    }

    void CombinedSpellSuggestions::replaceToSuggested(const QString &word, const QString &replacement) {
        size_t size = m_SpellSuggestions.size();
        LOG_INFO << size << "item(s)";
        bool anySucceeded = false;

        for (size_t i = 0; i < size; ++i) {
            auto &suggestionItem = m_SpellSuggestions.at(i);
            suggestionItem->replaceToSuggested(word, replacement);

            LOG_VERBOSE << i << "item's result is:" << suggestionItem->getReplacementSucceeded();

            if (suggestionItem->getReplacementSucceeded()) {
                anySucceeded = true;
            }
        }

        setReplacementSucceeded(anySucceeded);
    }
}
