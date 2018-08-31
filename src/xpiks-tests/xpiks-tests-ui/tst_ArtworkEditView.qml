import QtQuick 2.0
import QtTest 1.1
import xpiks 1.0
import XpiksTests 1.0
import "../../xpiks-qt/StackViews"
import "UiTestsStubPlugin"
import "TestUtils.js" as TestUtils

Item {
    id: root
    width: 800
    height: 600

    property string path: ''
    property bool isselected: false

    Component.onCompleted: TestsHost.bump()

    QtObject {
        id: keywordsWrapper
        property bool keywordsModel: false
    }

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
    }

    Loader {
        id: loader
        anchors.fill: parent
        asynchronous: true
        focus: true

        sourceComponent: ArtworkEditView {
            anchors.fill: parent
            artworkIndex: 0
            componentParent: root
        }
    }

    ClipboardHelper {
        id: clipboard
    }

    TestCase {
        id: testCase
        name: "ArtworkEdit"
        when: windowShown && (loader.status == Loader.Ready)
        property var titleEdit
        property var descriptionEdit
        property var keywordsEdit
        property var nextArtworkButton
        property var prevArtworkButton
        property var rosterListView
        property var copyLink
        property var editableTags
        property var artworkEditView: loader.item

        function initTestCase() {
            TestsHost.setup()

            artworkEditView.keywordsModel = artworkProxy.getBasicModelObject()

            titleEdit = findChild(artworkEditView, "titleTextInput")
            descriptionEdit = findChild(artworkEditView, "descriptionTextInput")
            keywordsEdit = findChild(artworkEditView, "nextTagTextInput")

            nextArtworkButton = findChild(artworkEditView, "selectNextButton")
            prevArtworkButton = findChild(artworkEditView, "selectPrevButton")

            rosterListView = findChild(artworkEditView, "rosterListView")

            copyLink = findChild(artworkEditView, "copyLink")

            editableTags = findChild(artworkEditView, "editableTags")
        }

        function cleanupTestCase() {
            TestsHost.cleanup()
        }

        function cleanup() {
            artworkProxy.clearModel()
            // assigning .text directly breaks binding
            keywordsEdit.remove(0, keywordsEdit.length)
            wait(500)
        }

        function getDelegateInstanceAt(contentItem, delegateObjectName, index) {
            for(var i = 0; i < contentItem.children.length; ++i) {
                var item = contentItem.children[i];
                // We have to check for the specific objectName we gave our
                // delegates above, since we also get some items that are not
                // our delegates here.
                if (item.objectName == delegateObjectName && item.delegateIndex == index)
                    return item;
            }
            return undefined;
        }

        function test_editTitleSimple() {
            titleEdit.forceActiveFocus()
            var testTitle = TestUtils.keyboardEnterSomething(testCase)
            compare(artworkProxy.title, testTitle)
        }

        function test_editDescriptionSimple() {
            descriptionEdit.forceActiveFocus()
            var testDescription = TestUtils.keyboardEnterSomething(testCase)
            compare(artworkProxy.description, testDescription)
        }

        function test_addKeywordsSmoke() {
            compare(artworkProxy.keywordsCount, 0)

            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            compare(artworkProxy.keywordsCount, 1)
            compare(artworkProxy.getKeywordsString(), testKeyword)
        }

        function test_switchCurrentArtworkButtons() {
            titleEdit.forceActiveFocus()
            var testTitle = TestUtils.keyboardEnterSomething(testCase)
            descriptionEdit.forceActiveFocus()
            var testDescription = TestUtils.keyboardEnterSomething(testCase)

            compare(rosterListView.currentIndex, 0)
            compare(artworkProxy.description, testDescription)
            compare(artworkProxy.title, testTitle)

            mouseClick(nextArtworkButton)

            compare(rosterListView.currentIndex, 1)
            console.log(artworkProxy.description)
            verify(artworkProxy.description !== testDescription)
            verify(artworkProxy.title !== testTitle)

            mouseClick(prevArtworkButton)

            compare(rosterListView.currentIndex, 0)
            compare(artworkProxy.description, testDescription)
            compare(artworkProxy.title, testTitle)
        }

        function test_switchCurrentArtworkMouseClick() {
            titleEdit.forceActiveFocus()
            var testTitle = TestUtils.keyboardEnterSomething(testCase)
            descriptionEdit.forceActiveFocus()
            var testDescription = TestUtils.keyboardEnterSomething(testCase)

            compare(rosterListView.currentIndex, 0)
            compare(artworkProxy.description, testDescription)
            compare(artworkProxy.title, testTitle)

            var nextItem = getDelegateInstanceAt(rosterListView.contentItem,
                                                 "rosterDelegateItem",
                                                 rosterListView.currentIndex + 1)
            mouseClick(nextItem)

            compare(rosterListView.currentIndex, 1)
            console.log(artworkProxy.description)
            verify(artworkProxy.description !== testDescription)
            verify(artworkProxy.title !== testTitle)

            var prevItem = getDelegateInstanceAt(rosterListView.contentItem,
                                                 "rosterDelegateItem",
                                                 rosterListView.currentIndex - 1)
            mouseClick(prevItem)

            compare(rosterListView.currentIndex, 0)
            compare(artworkProxy.description, testDescription)
            compare(artworkProxy.title, testTitle)
        }

        function test_copyFromMoreLink() {
            compare(copyLink.enabled, false)

            keywordsEdit.forceActiveFocus()
            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)
            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)
            // do not press comma - test adding keyword on losing focus

            compare(copyLink.enabled, true)
            copyLink.forceActiveFocus()

            mouseClick(copyLink)

            compare(clipboard.getText(), testKeyword1 + ", " + testKeyword2)
        }

        function test_autoCompleteKeywordBasic() {
            verify(typeof artworkEditView.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_W)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_A)

            wait(200)

            verify(typeof artworkEditView.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_H)
            keyClick(Qt.Key_E)

            wait(200)

            keyClick(Qt.Key_Down)
            wait(200)

            keyClick(Qt.Key_Return)
            wait(200)

            compare(artworkProxy.keywordsCount, 1)
            compare(artworkProxy.getKeywordsString(), "weather")
        }

        function test_autoCompleteCancelWhenShortText() {
            verify(typeof artworkEditView.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_R)
            keyClick(Qt.Key_U)

            wait(200)

            verify(typeof artworkEditView.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Backspace)

            wait(200)

            verify(typeof artworkEditView.autoCompleteBox === "undefined")
        }

        function test_autoCompletePreset() {
            verify(typeof artworkEditView.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_I)
            keyClick(Qt.Key_N)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_R)
            keyClick(Qt.Key_F)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)

            wait(200)

            verify(typeof artworkEditView.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Down)
            wait(200)

            keyClick(Qt.Key_Return, Qt.ControlModifier)
            wait(200)

            compare(artworkProxy.keywordsCount, 3)
            compare(artworkProxy.getKeywordsString(), "some, other, keywords")
        }

        function test_autoCompleteIntoNonEmptyEdit() {
            verify(typeof artworkEditView.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_I)
            keyClick(Qt.Key_N)
            keyClick(Qt.Key_Space)

            keyClick(Qt.Key_S)
            keyClick(Qt.Key_P)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)

            wait(200)

            keyClick(Qt.Key_Down)
            wait(200)

            keyClick(Qt.Key_Return)
            wait(200)

            compare(keywordsEdit.text, "in space")
        }

        function test_spellingIsCheckedForWrong() {
            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(200)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            tryCompare(keywordWrapper, "hasSpellCheckError", true, 2000)
        }

        function test_spellingIsCheckedForCorrect() {
            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_I)
            keyClick(Qt.Key_N)
            keyClick(Qt.Key_Space)

            keyClick(Qt.Key_S)
            keyClick(Qt.Key_P)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_Comma)

            wait(200)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            wait(1500)

            compare(keywordWrapper.hasSpellCheckError, false)
        }

        function test_duplicateIsDetectedWithDescription() {
            descriptionEdit.forceActiveFocus()
            keyClick(Qt.Key_S)
            keyClick(Qt.Key_P)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)
            keyClick(Qt.Key_E)

            keywordsEdit.forceActiveFocus()
            keyClick(Qt.Key_S)
            keyClick(Qt.Key_P)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)
            keyClick(Qt.Key_E)

            keyClick(Qt.Key_Comma)

            wait(200)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            tryCompare(keywordWrapper, "hasDuplicate", true, 2000)
            compare(keywordWrapper.hasSpellCheckError, false)
        }

        function test_doubleClickEditsKeyword() {
            keywordsEdit.forceActiveFocus()
            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(200)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            compare(artworkProxy.getKeywordsString(), testKeyword1)
            mouseDoubleClick(keywordWrapper)

            wait(200)

            for (var i = 0; i < testKeyword1.length; i++) {
                keyClick(Qt.Key_Backspace)
            }

            wait(200)

            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Enter)

            wait(200)

            compare(artworkProxy.getKeywordsString(), testKeyword2)
        }

        function test_editInPlainText() {
            keywordsEdit.forceActiveFocus()
            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(500)

            artworkEditView.editInPlainText()

            wait(500)

            // hack to detect if plain text edit hasn't started
            keyClick(Qt.Key_Comma)
            keyClick(Qt.Key_Backspace)

            keyClick(Qt.Key_Comma)
            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)

            wait(200)

            keyClick(Qt.Key_Enter, Qt.ControlModifier)

            wait(200)

            compare(artworkProxy.getKeywordsString(), testKeyword1 + ", " + testKeyword2)
        }
    }
}
