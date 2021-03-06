/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CURRENTITEMARTWORK_H
#define CURRENTITEMARTWORK_H

#include <cstddef>
#include <memory>

#include <QString>
#include <QStringList>

#include "Artworks/artworkssnapshot.h"
#include "Commands/Base/icommandtemplate.h"
#include "Common/types.h"
#include "KeywordsPresets/presetmodel.h"
#include "Models/Editing/icurrenteditable.h"

namespace Commands {
    class ICommand;
}
namespace KeywordsPresets {
    class IPresetsManager;
}

namespace Artworks {
    class ArtworkMetadata;
}

namespace Models {
    class CurrentEditableArtwork : public ICurrentEditable
    {
        using IArtworksCommandTemplate = Commands::ICommandTemplate<Artworks::ArtworksSnapshot>;

    public:
        CurrentEditableArtwork(std::shared_ptr<Artworks::ArtworkMetadata> const &artworkMetadata,
                               std::shared_ptr<IArtworksCommandTemplate> const &updateTemplate);

    public:
        size_t getOriginalIndex() const;

        // ICurrentEditable interface
    public:
        virtual Common::ID_t getItemID() override;
        virtual QString getTitle() override;
        virtual QString getDescription() override;
        virtual QStringList getKeywords() override;
        virtual bool hasKeywords(const QStringList &keywordsList) override;

        virtual std::shared_ptr<Commands::ICommand> setTitle(const QString &value) override;
        virtual std::shared_ptr<Commands::ICommand> setDescription(const QString &value) override;
        virtual std::shared_ptr<Commands::ICommand> setKeywords(const QStringList &keywords) override;

        virtual std::shared_ptr<Commands::ICommand> appendKeywords(const QStringList &keywords) override;

        virtual std::shared_ptr<Commands::ICommand> appendPreset(KeywordsPresets::ID_t presetID,
                                                                 KeywordsPresets::IPresetsManager &presetsManager) override;
        virtual std::shared_ptr<Commands::ICommand> expandPreset(int keywordIndex,
                                                                 KeywordsPresets::ID_t presetID,
                                                                 KeywordsPresets::IPresetsManager &presetsManager) override;
        virtual std::shared_ptr<Commands::ICommand> removePreset(KeywordsPresets::ID_t presetID,
                                                                 KeywordsPresets::IPresetsManager &presetsManager) override;

        virtual std::shared_ptr<Commands::ICommand> update() override;

    public:
        virtual std::shared_ptr<Commands::ICommand> applyEdits(const QString &title,
                                                               const QString &description,
                                                               const QStringList &keywords) override;

    private:
        std::shared_ptr<Artworks::ArtworkMetadata> m_ArtworkMetadata;
        std::shared_ptr<IArtworksCommandTemplate> m_UpdateTemplate;
    };
}

#endif // CURRENTITEMARTWORK_H
