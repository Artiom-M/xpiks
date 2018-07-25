/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHANGESLISTENER_H
#define CHANGESLISTENER_H

#include <vector>
#include <functional>

namespace Common {
    // tag for events with same underlying type
    struct MessageType {
        enum Type {
            SpellCheck
        };
    };

    template<typename T>
    class MessagesTarget {
    public:
        virtual ~MessagesTarget() {}
        virtual void handleMessage(const T &message) = 0;
    };

    template<typename T>
    class MessagesSource {
    public:
        virtual ~MessagesSource() {}

    public:
        void addTarget(MessagesTarget<T> &target) {
            m_Targets.push_back(target);
        }

        void sendMessage(const T &message) const {
            for (auto &target: m_Targets) {
                target.get().handleMessage(message);
            }
        }

    private:
        std::vector<std::reference_wrapper<MessagesTarget<T>>> m_Targets;
    };

    template<typename T>
    void connectTarget(MessagesTarget<T> &t,
                       std::initializer_list<std::reference_wrapper<MessagesSource<T>>> sources) {
        for (auto s: sources) {
            s.addTarget(t);
        }
    }
}

#endif // CHANGESLISTENER_H
