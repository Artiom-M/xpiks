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

import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import xpiks 1.0
import "../Constants"
import "../Constants/Colors.js" as Colors
import "../StyledControls"

Flickable {
    id: flowListView
    anchors.fill: parent
    anchors.leftMargin: 5
    anchors.topMargin: 5
    anchors.bottomMargin: 5
    contentWidth: parent.width
    contentHeight: flow.childrenRect.height
    boundsBehavior: Flickable.StopAtBounds
    flickableDirection: Flickable.VerticalFlick
    rightMargin: 10
    interactive: false
    clip: true

    property int scrollStep: 10
    property alias count: repeater.count
    property int currentIndex: -1
    property variant currentItem;
    property alias delegate: repeater.delegate
    property alias flow: flow.flow
    property alias model: repeater.model
    property alias isFocused: nextTagTextInput.activeFocus
    property alias editEnabled: editWrapper.enabled
    property int flowSpacing: 5
    property bool stealWheel: true

    signal tagAdded(string text)
    signal removeLast()
    signal focusLost()
    signal tagsPasted(var tagsList)
    signal copyRequest();
    signal backTabPressed();
    signal tabPressed();

    function activateEdit() {
        if (editEnabled && !nextTagTextInput.activeFocus) {
            scrollToBottom()
            nextTagTextInput.forceActiveFocus()
        }
    }

    function getEditedText() {
        var tagText = nextTagTextInput.text;
        return helpersWrapper.sanitizeKeyword(tagText);
    }

    function raiseAddTag(text) {
        var canBeAdded = helpersWrapper.isKeywordValid(text);
        if (canBeAdded) {
            tagAdded(text);
        }
        return canBeAdded;
    }

    function scrollDown () {
        var flickable = flowListView;
        if (flowListView.contentHeight >= flowListView.height) {
            flickable.contentY = Math.min (flickable.contentY + scrollStep + flowSpacing, flickable.contentHeight - flickable.height);

            var lines = Math.floor(flickable.contentY / (scrollStep + flowSpacing));
            flickable.contentY = lines*(scrollStep + flowSpacing)
        }
    }

    function scrollUp () {
        var flickable = flowListView;
        if (flowListView.contentHeight >= flowListView.height) {
            flickable.contentY = Math.max (flickable.contentY - scrollStep - flowSpacing, 0);

            var lines = Math.floor(flickable.contentY / (scrollStep + flowSpacing));
            flickable.contentY = lines*(scrollStep + flowSpacing)
        }
    }

    function scrollToBottom() {
        if (flowListView.contentHeight >= flowListView.height) {
            flowListView.contentY = flowListView.contentHeight - flowListView.height
        }
    }

    function adjustEdit() {
        if (Math.abs(editWrapper.y - flowListView.contentY) <= flowListView.height)  {
            flowListView.scrollToBottom()
        }
    }

    function forceRefreshRepeater() {
        repeater.update()
    }

    function removeFocus() {
        nextTagTextInput.focus = false
    }

    MouseArea {
        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width
        height: flowListView.contentHeight > flowListView.height ? flowListView.contentHeight : flowListView.height
        onClicked: {
            activateEdit()
            mouse.accepted = false
        }

        propagateComposedEvents: true
        preventStealing: true

        onWheel: {
            if (!stealWheel && !nextTagTextInput.activeFocus || flowListView.height >= flowListView.contentHeight) {
                wheel.accepted = false
                return
            }

            if (wheel.angleDelta.y < 0) {
                var maxScrollPos = flowListView.contentHeight - flowListView.height
                if (Math.abs(flowListView.contentY - maxScrollPos) > scrollStep) {
                    scrollDown()
                } else {
                    scrollToBottom()
                    wheel.accepted = false
                }
            } else {
                if (flowListView.contentY > scrollStep) {
                    scrollUp()
                } else {
                    flowListView.contentY = 0
                    wheel.accepted = false
                }
            }
        }
    }

    Flow {
        id: flow
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.left: parent.left
        spacing: flowSpacing

        property real lastHeight

        /*add: Transition {
            NumberAnimation { properties: "x,y"; easing.type: Easing.Bezier }
        }*/

        populate: Transition {
            NumberAnimation { properties: "x,y"; from: 200; duration: 100; easing.type: Easing.OutBounce }
        }

        /*move: Transition {
            NumberAnimation { properties: "x,y"; easing.type: Easing.OutBounce }
        }*/

        onHeightChanged: {
            if (!lastHeight) {
                lastHeight = height
            }

            if (height > lastHeight) {
                flowListView.scrollToBottom()
            }

            adjustEdit()

            lastHeight = height
        }

        Repeater {
            id: repeater

            onItemRemoved: {
                adjustEdit()
            }
        }

        Item {
            id: editWrapper
            width: 100
            height: 20*settingsModel.keywordSizeScale

            TextInput {
                id: nextTagTextInput
                maximumLength: 30
                selectedTextColor: Colors.defaultLightColor
                selectionColor: Colors.defaultControlColor
                selectByMouse: true
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                color: Colors.defaultLightColor
                font.family: "Helvetica"
                font.pixelSize: 12*settingsModel.keywordSizeScale
                verticalAlignment: TextInput.AlignVCenter
                renderType: TextInput.NativeRendering
                focus: true

                /*validator: RegExpValidator {
                    // copy paste in keys.onpressed Paste
                    regExp: /^(?:\c+(?:-| |$))+$/
                }*/

                //onFocusChanged: focusLost()

                onEditingFinished: {
                    var tagText = getEditedText();
                    if (raiseAddTag(tagText)) {
                        nextTagTextInput.text = '';
                    }

                    focusLost()
                }

                Keys.onPressed: {
                    if(event.matches(StandardKey.Paste)) {
                        var clipboardText = clipboard.getText();
                        clipboardText = clipboardText.replace(/(\r\n|\n|\r)/gm, '');
                        var keywordsToAdd = [];

                        var words = clipboardText.split(/,|;/);

                        if (words.length > 1) {
                            for (var i = 0; i < words.length; i++) {
                                var sanitizedTagText = helpersWrapper.sanitizeKeyword(words[i]);
                                if (helpersWrapper.isKeywordValid(sanitizedTagText)) {
                                    keywordsToAdd.push(sanitizedTagText);
                                }
                            }

                            tagsPasted(keywordsToAdd);
                        } else {
                            nextTagTextInput.text = words[0]
                        }

                        event.accepted = true;
                    }
                    else if (event.matches(StandardKey.Copy)) {
                        copyRequest()
                        event.accepted = true
                    }
                    else if (event.key === Qt.Key_Comma) {
                        var tagText = getEditedText();
                        if (raiseAddTag(tagText)) {
                            nextTagTextInput.text = '';
                        }

                        event.accepted = true;
                    }
                    else if (event.key === Qt.Key_Backspace) {
                        if (nextTagTextInput.length == 0) {
                            removeLast();
                            event.accepted = true;
                        }
                    }
                    else if (event.key === Qt.Key_Tab) {
                        tabPressed()
                        event.accepted = true;
                    }
                    else if (event.key === Qt.Key_Backtab) {
                        backTabPressed()
                        event.accepted = true;
                    }

                    scrollToBottom()
                }
            }
        }

        ClipboardHelper {
            id: clipboard
        }
    }
}