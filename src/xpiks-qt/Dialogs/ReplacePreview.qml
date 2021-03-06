/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.2
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import xpiks 1.0
import "../Constants"
import "../Common.js" as Common;
import "../Components"
import "../StyledControls"
import "../Constants/UIConfig.js" as UIConfig

Item {
    id: replacePreviewComponent
    anchors.fill: parent
    property variant componentParent
    property variant replaceModel: dispatcher.getCommandTarget(UICommand.FindAndReplaceInSelected)
    property bool isRestricted: false

    signal dialogDestruction();
    Component.onDestruction: dialogDestruction();

    function closePopup() {
        replaceModel.clearArtworks()
        replacePreviewComponent.destroy()
    }

    Component.onCompleted: focus = true
    Keys.onEscapePressed: closePopup()
    Keys.onReturnPressed: replaceModel.replace()
    Keys.onEnterPressed: replaceModel.replace()

    Connections {
        target: replaceModel
        onReplaceSucceeded: closePopup()
    }

    PropertyAnimation { target: replacePreviewComponent; property: "opacity";
        duration: 400; from: 0; to: 1;
        easing.type: Easing.InOutQuad ; running: true }

    // This rectange is the a overlay to partially show the parent through it
    // and clicking outside of the 'dialog' popup will do 'nothing'
    Rectangle {
        anchors.fill: parent
        id: overlay
        color: "#000000"
        opacity: 0.6
        // add a mouse area so that clicks outside
        // the dialog window will not do anything
        MouseArea {
            anchors.fill: parent
        }
    }

    FocusScope {
        anchors.fill: parent

        MouseArea {
            anchors.fill: parent
            onWheel: wheel.accepted = true
            onClicked: mouse.accepted = true
            onDoubleClicked: mouse.accepted = true
            property real old_x : 0
            property real old_y : 0

            onPressed:{
                var tmp = mapToItem(replacePreviewComponent, mouse.x, mouse.y);
                old_x = tmp.x;
                old_y = tmp.y;

                var dialogPoint = mapToItem(dialogWindow, mouse.x, mouse.y);
                if (!Common.isInComponent(dialogPoint, dialogWindow)) {
                    closePopup()
                }
            }

            onPositionChanged: {
                var old_xy = Common.movePopupInsideComponent(replacePreviewComponent, dialogWindow, mouse, old_x, old_y);
                old_x = old_xy[0]; old_y = old_xy[1];
            }
        }

        RectangularGlow {
            anchors.fill: dialogWindow
            anchors.topMargin: glowRadius/2
            anchors.bottomMargin: -glowRadius/2
            glowRadius: 4
            spread: 0.0
            color: uiColors.popupGlowColor
            cornerRadius: glowRadius
        }

        // This rectangle is the actual popup
        Rectangle {
            id: dialogWindow
            width: 700
            height: 580
            color: uiColors.popupBackgroundColor
            anchors.centerIn: parent
            Component.onCompleted: anchors.centerIn = undefined

            ColumnLayout {
                spacing: 10
                anchors.fill: parent
                anchors.margins: 20

                RowLayout {
                    spacing: 5

                    StyledText {
                        text: i18.n + qsTr("Replace", "caption")
                    }

                    StyledText {
                        text: '"' + replaceModel.replaceFrom + '"'
                    }

                    StyledText {
                        text: i18.n + qsTr("with")
                    }

                    StyledText {
                        text: '"' + replaceModel.replaceTo + '"'
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: uiColors.defaultControlColor

                    StyledScrollView {
                        anchors.fill: parent
                        anchors.margins: 10

                        ListView {
                            id: replacePreviewList
                            model: replaceModel
                            spacing: 5

                            delegate: Rectangle {
                                id: imageWrapper
                                property int delegateIndex: index
                                color: isselected ? uiColors.selectedArtworkBackground : uiColors.artworkBackground
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.rightMargin: 10
                                height: columnRectangle.height

                                StyledCheckbox {
                                    id: applyReplaceCheckBox
                                    anchors.leftMargin: 14
                                    anchors.left: parent.left
                                    width: 20
                                    anchors.verticalCenter: parent.verticalCenter
                                    onClicked: editisselected = checked
                                    Component.onCompleted: applyReplaceCheckBox.checked = isselected

                                    Connections {
                                        target: replaceModel
                                        onAllSelectedChanged: {
                                            applyReplaceCheckBox.checked = isselected
                                        }
                                    }
                                }

                                Item {
                                    id: imageItem
                                    anchors.leftMargin: 0
                                    anchors.left: applyReplaceCheckBox.right
                                    anchors.top: parent.top
                                    width: 120
                                    height: parent.height

                                    ColumnLayout {
                                        anchors.centerIn: parent
                                        anchors.verticalCenterOffset: 7
                                        spacing: 7
                                        width: 110

                                        Item {
                                            id: imageContainer
                                            width: 100
                                            height: 100
                                            anchors.horizontalCenter: parent.horizontalCenter

                                            Image {
                                                id: artworkImage
                                                anchors.fill: parent
                                                source: "image://cached/" + path
                                                sourceSize.width: 150
                                                sourceSize.height: 150
                                                fillMode: settingsModel.fitSmallPreview ? Image.PreserveAspectFit : Image.PreserveAspectCrop
                                                asynchronous: true
                                                cache: false
                                            }

                                            Image {
                                                id: videoTypeIconSmall
                                                visible: isvideo
                                                enabled: isvideo
                                                source: "qrc:/Graphics/video-icon-s.png"
                                                fillMode: Image.PreserveAspectFit
                                                sourceSize.width: 150
                                                sourceSize.height: 150
                                                anchors.fill: artworkImage
                                                cache: true
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: {
                                                    editisselected = !isselected
                                                    applyReplaceCheckBox.checked = isselected
                                                }
                                            }
                                        }

                                        StyledText {
                                            Layout.fillWidth: true
                                            elide: Text.ElideMiddle
                                            horizontalAlignment: Text.AlignHCenter
                                            text: path.split(/[\\/]/).pop()
                                            font.pixelSize: UIConfig.fontPixelSize
                                        }

                                        Item {
                                            Layout.fillHeight: true
                                        }
                                    }
                                }

                                Rectangle {
                                    id: columnRectangle
                                    anchors.left: imageItem.right
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    height: (childrenRect.height < 130) ? 150 : (childrenRect.height + 20)
                                    color: isselected ? uiColors.selectedArtworkBackground : uiColors.artworkBackground

                                    Item {
                                        clip: true
                                        anchors.left: parent.left
                                        anchors.top: parent.top
                                        anchors.right: parent.right
                                        anchors.leftMargin: 5
                                        anchors.rightMargin: 10
                                        anchors.topMargin: 10
                                        height: childrenRect.height

                                        StyledText {
                                            id: titleHit
                                            text: i18.n + qsTr("Title:")
                                            isActive: isselected
                                            anchors.left: parent.left
                                            anchors.top: parent.top
                                        }

                                        Rectangle {
                                            id: titleRectangle
                                            height: childrenRect.height + 10
                                            color: (isselected && enabled) ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                                            anchors.right: parent.right
                                            anchors.left: parent.left
                                            anchors.top: titleHit.bottom
                                            anchors.topMargin: 3
                                            enabled: hastitle
                                            opacity: (isselected && enabled) ? 1 : 0.5

                                            StyledTextEdit {
                                                id: titleText
                                                anchors.top: parent.top
                                                anchors.topMargin: 5
                                                anchors.right: parent.right
                                                anchors.left: parent.left
                                                anchors.leftMargin: 5
                                                anchors.rightMargin: 5
                                                wrapMode: TextEdit.Wrap
                                                text: i18.n + replaceModel.getSearchTitle(imageWrapper.delegateIndex)
                                                readOnly: true
                                                selectByKeyboard: false
                                                selectByMouse: false
                                                isActive: isselected
                                                Component.onCompleted: {
                                                    replaceModel.initHighlighting(titleText.textDocument)
                                                }
                                            }
                                        }

                                        StyledText {
                                            id: descriptionHit
                                            text: i18.n + qsTr("Description:")
                                            isActive: isselected
                                            anchors.left: parent.left
                                            anchors.top: titleRectangle.bottom
                                            anchors.topMargin: 10
                                        }

                                        Rectangle {
                                            id: descriptionRectangle
                                            color: (isselected && enabled) ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                                            height: childrenRect.height + 10
                                            anchors.top: descriptionHit.bottom
                                            anchors.topMargin: 3
                                            anchors.right: parent.right
                                            anchors.left: parent.left
                                            enabled: hasdescription
                                            opacity: (isselected && enabled) ? 1 : 0.5

                                            StyledTextEdit {
                                                id: descriptionText
                                                wrapMode: TextEdit.Wrap
                                                anchors.top: parent.top
                                                anchors.topMargin: 5
                                                anchors.right: parent.right
                                                anchors.left: parent.left
                                                anchors.leftMargin: 5
                                                anchors.rightMargin: 5
                                                text: i18.n + replaceModel.getSearchDescription(imageWrapper.delegateIndex).trim()
                                                readOnly: true
                                                selectByKeyboard: false
                                                selectByMouse: false
                                                isActive: isselected
                                                Component.onCompleted: {
                                                    replaceModel.initHighlighting(descriptionText.textDocument)
                                                }
                                            }
                                        }

                                        StyledText {
                                            id: keywordsHit
                                            text: i18.n + qsTr("Keywords:")
                                            isActive: isselected
                                            anchors.left: parent.left
                                            anchors.top: descriptionRectangle.bottom
                                            anchors.topMargin: 10
                                        }

                                        Rectangle {
                                            id: keywordsRectangle
                                            color: (isselected && enabled) ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                                            height: childrenRect.height + 10
                                            anchors.right: parent.right
                                            anchors.left: parent.left
                                            enabled: haskeywords
                                            anchors.top: keywordsHit.bottom
                                            anchors.topMargin: 3
                                            opacity: (isselected && enabled) ? 1 : 0.5

                                            StyledTextEdit {
                                                id: keywordsText
                                                anchors.top: parent.top
                                                anchors.topMargin: 5
                                                anchors.right: parent.right
                                                anchors.left: parent.left
                                                anchors.leftMargin: 5
                                                anchors.rightMargin: 5
                                                wrapMode: TextEdit.Wrap
                                                text: i18.n + replaceModel.getSearchKeywords(imageWrapper.delegateIndex)
                                                readOnly: true
                                                selectByKeyboard: false
                                                selectByMouse: false
                                                isActive: isselected

                                                Component.onCompleted: {
                                                    replaceModel.initHighlighting(keywordsText.textDocument)
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        anchors.fill: parent
                        visible: replaceModel.count == 0

                        StyledText {
                            text: i18.n + qsTr("Nothing found")
                            anchors.centerIn: parent
                            color: uiColors.selectedArtworkBackground
                        }
                    }
                }

                Item {
                    height: 1
                }

                RowLayout {
                    spacing: 20
                    height: 24

                    StyledButton {
                        text: i18.n + qsTr("Select all")
                        width: 100
                        enabled: replaceModel.count > 0
                        onClicked: {
                            replaceModel.selectAll();
                        }
                    }

                    StyledButton {
                        text: i18.n + qsTr("Unselect all")
                        width: 100
                        enabled: replaceModel.count > 0
                        onClicked: {
                            replaceModel.unselectAll();
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    StyledButton {
                        text: i18.n + qsTr("Replace", "button")
                        enabled: replaceModel.count > 0
                        width: 100
                        onClicked: {
                            replaceModel.replace()
                        }
                    }

                    StyledButton {
                        text: i18.n + qsTr("Close")
                        width: 100
                        onClicked: {
                            closePopup()
                        }
                    }
                }
            }
        }
    }
}
