TEMPLATE = app

QT += qml quick
CONFIG += qmltestcase plugin c++11
TARGET = xpiks-tests-ui

DEFINES += QML_IMPORT_TRACE
DEFINES += QT_DEBUG_PLUGINS

#DEFINES += QT_FATAL_WARNINGS

SOURCES += main.cpp \
    ../../xpiks-qt/QMLExtensions/triangleelement.cpp

STUB_PLUGIN_NAME = UiTestsStubPlugin

DEFINES += PLUGIN_STUB_IMPORT_DIR="$$PWD"

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = "$$PWD/$$STUB_PLUGIN_NAME"

# Default rules for deployment.
include(deployment.pri)

DISTFILES += \
    ../../xpiks-qt/Components/EditableTags.qml \
    tst_EditableTags.qml \
    FakeColors.qml \
    ../../xpiks-qt/StackViews/CombinedEditView.qml \
    tst_CombinedEditView.qml \
    tst_ArtworkEditView.qml \
    tst_CsvExportDialog.qml \
    ../../xpiks-qt/Dialogs/CsvExportDialog.qml \
    tst_UploadDialog.qml \
    tst_PresetsEditDialog.qml

HEADERS += \
    ../../xpiks-qt/Helpers/clipboardhelper.h \
    ../../xpiks-qt/QMLExtensions/triangleelement.h

RESOURCES += \
    xpiks-tests-ui.qrc
