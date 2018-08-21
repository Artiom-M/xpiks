import QtQuick 2.0
import QtTest 1.1
import XpiksTests 1.0
import "../../xpiks-qt/Dialogs"

Item {
    id: root
    width: 800
    height: 600
    property string scoreme: TestsHost.scoreme

    QtObject {
        id: uiManager
        property real keywordHeight: 10
    }

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
    }

    QtObject {
        id: settingsModel
        property int keywordSizeScale: 1
    }

    ListModel {
        id: columnsModel

        ListElement { property: 1; column: "test" }
        ListElement { property: 2; column: "filename" }

        function getPropertiesList() { return {} }
    }

    ListModel {
        id: csvExportModel

        property bool isExporting: false
        property int artworksCount: 0
        property string outputDirectory: ""

        ListElement { name: "Jane"; isselected: false; issystem: false }
        ListElement { name: "Harry"; isselected: false; issystem: false }
        ListElement { name: "Wendy"; isselected: false; issystem: false }

        function startExport() {}
        function setCurrentItem() {}
        function getColumnsModel() { return columnsModel; }
        function requestSave() {}

        signal exportFinished()
    }

    QtObject {
        id: i18
        property string n: ""
    }

    CsvExportDialog {
        id: exportDialog
        anchors.fill: parent
    }

    TestCase {
        name: "CsvExport"
        when: windowShown
    }
}
