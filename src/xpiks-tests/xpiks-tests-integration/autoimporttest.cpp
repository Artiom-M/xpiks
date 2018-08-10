#include "autoimporttest.h"
#include <QUrl>
#include <QFileInfo>
#include <QStringList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString AutoImportTest::testName() {
    return QLatin1String("AutoImportTest");
}

void AutoImportTest::setup() {
    m_TestsApp.setAutoFindVector(false);
    m_TestsApp.setUseAutoImport(true);
}

int AutoImportTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/read-only/026.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    QStringList expectedKeywords = QString("abstract,art,black,blue,creative,dark,decor,decoration,decorative,design,dot,drops,elegance,element,geometric,interior,light,modern,old,ornate,paper,pattern,purple,retro,seamless,style,textile,texture,vector,wall,wallpaper").split(',');

    VERIFY(expectedKeywords == m_TestsApp.getArtwork(0)->getKeywords(), "Keywords are not the same after first import!");

    artItemsModel->removeArtworks({{0, 0}});

    VERIFY(m_TestsApp.undoLastAction(), "Failed to Undo last action");

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata after undo.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");
    VERIFY(ioCoordinator->getImportIDs().size() == 2, "Undo import does not have any trace");

    VERIFY(expectedKeywords == artItemsModel->getArtwork(0)->getKeywords(), "Keywords are not the same after undo import!");

    return 0;
}

