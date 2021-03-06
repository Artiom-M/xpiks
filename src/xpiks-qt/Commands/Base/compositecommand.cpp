/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "compositecommand.h"

namespace Commands {
    void CompositeCommand::execute() {
        for (auto &command: m_Commands) {
            command->execute();
        }
    }

    void CompositeCommand::undo() {
        for (auto &command: m_Commands) {
            if (command->canUndo()) {
                command->execute();
            }
        }
    }

    bool CompositeCommand::canUndo() {
        bool anyUndo = false;
        for (auto &command: m_Commands) {
            if (command->canUndo()) {
                anyUndo = true;
                break;
            }
        }
        return anyUndo;
    }
}
