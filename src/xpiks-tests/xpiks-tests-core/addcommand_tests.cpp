#include <QStringList>
#include <QSignalSpy>
#include "addcommand_tests.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/artitemsmodelmock.h"
#include "Mocks/artworksrepositorymock.h"
#include "Mocks/coretestsenvironment.h"
#include "../../xpiks-qt/Commands/addartworkscommand.h"
#include "../../xpiks-qt/Models/settingsmodel.h"

#define DECLARE_MODELS \
    Mocks::CommandManagerMock commandManagerMock;\
    Mocks::ArtItemsModelMock artItemsMock;\
    Mocks::CoreTestsEnvironment environment;\
    Mocks::ArtworksRepositoryMock artworksRepository(environment);\
    commandManagerMock.InjectDependency(&artworksRepository);\
    Mocks::ArtItemsModelMock *artItemsModel = &artItemsMock;\
    commandManagerMock.InjectDependency(artItemsModel);

void AddCommandTests::addNoArtworksToEmptyRepositoryTest() {
    DECLARE_MODELS;

    QSignalSpy artItemsBeginInsertSpy(&artItemsMock, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy artItemsEndInsertSpy(&artItemsMock, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy artworkRepoBeginInsertSpy(&artworksRepository, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy artworkRepoEndInsertSpy(&artworksRepository, SIGNAL(rowsInserted(QModelIndex,int,int)));

    QStringList filenames;

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, QStringList(), 0));
    auto result = commandManagerMock.processCommand(addArtworksCommand);
    auto addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    int newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, 0);

    QCOMPARE(artItemsBeginInsertSpy.count(), 0);
    QCOMPARE(artItemsEndInsertSpy.count(), 0);
    QCOMPARE(artworkRepoBeginInsertSpy.count(), 0);
    QCOMPARE(artworkRepoEndInsertSpy.count(), 0);
}

void AddCommandTests::addOneArtworkToEmptyRepositoryTest() {
    DECLARE_MODELS;

    QSignalSpy artItemsBeginInsertSpy(&artItemsMock, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy artItemsEndInsertSpy(&artItemsMock, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy artworkRepoBeginInsertSpy(&artworksRepository, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy artworkRepoEndInsertSpy(&artworksRepository, SIGNAL(rowsInserted(QModelIndex,int,int)));

    QStringList filenames;
    filenames.append("somefile.jpg");

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, QStringList(), 0));
    auto result = commandManagerMock.processCommand(addArtworksCommand);
    auto addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    int newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, 1);

    QCOMPARE(artItemsBeginInsertSpy.count(), 1);
    QList<QVariant> artItemsSpyArguments = artItemsBeginInsertSpy.takeFirst();
    QCOMPARE(artItemsSpyArguments.at(1).toInt(), 0);
    QCOMPARE(artItemsSpyArguments.at(2).toInt(), 0);

    QCOMPARE(artItemsEndInsertSpy.count(), 1);
    artItemsSpyArguments = artItemsEndInsertSpy.takeFirst();
    QCOMPARE(artItemsSpyArguments.at(1).toInt(), 0);
    QCOMPARE(artItemsSpyArguments.at(2).toInt(), 0);

    QCOMPARE(artworkRepoBeginInsertSpy.count(), 1);
    QList<QVariant> artworkRepSpyArgs = artworkRepoBeginInsertSpy.takeFirst();
    QCOMPARE(artworkRepSpyArgs.at(1).toInt(), 0);
    QCOMPARE(artworkRepSpyArgs.at(2).toInt(), 0);

    QCOMPARE(artworkRepoEndInsertSpy.count(), 1);
    artworkRepSpyArgs = artworkRepoEndInsertSpy.takeFirst();
    QCOMPARE(artworkRepSpyArgs.at(1).toInt(), 0);
    QCOMPARE(artworkRepSpyArgs.at(2).toInt(), 0);
}

void AddCommandTests::addAndAttachVectorsTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile.jpg" << "/another/path/to/some/other/file.jpg";
    vectors << "/path/to/somefile.eps" << "/another/path/to/some/other/file.eps";

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, vectors, 0));
    auto result = commandManagerMock.processCommand(addArtworksCommand);
    auto addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    int newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, 2);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(artItemsModel->getMockArtwork(i)->hasVectorAttached());
    }
}

void AddCommandTests::addAndAttachVectorsLaterTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile.jpg" << "/another/path/to/some/other/file.jpg";
    vectors << "/path/to/somefile.eps" << "/another/path/to/some/other/file.eps";

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, QStringList(), 0));
    auto result = commandManagerMock.processCommand(addArtworksCommand);
    auto addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    int newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, 2);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(!artItemsModel->getMockArtwork(i)->hasVectorAttached());
    }

    std::shared_ptr<Commands::AddArtworksCommand> anotherAddArtworksCommand(new Commands::AddArtworksCommand(QStringList(), vectors, 0));
    result = commandManagerMock.processCommand(anotherAddArtworksCommand);
    addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, 0);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(artItemsModel->getMockArtwork(i)->hasVectorAttached());
    }
}

void AddCommandTests::addAndDontAttachVectorsOtherDirTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile.jpg" << "/another/path/to/some/other/file.jpg";
    vectors << "/another/path/to/somefile.eps" << "/path/to/some/other/file.eps";

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, vectors, 0));
    auto result = commandManagerMock.processCommand(addArtworksCommand);
    auto addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    int newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, 2);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(!artItemsModel->getMockArtwork(i)->hasVectorAttached());
    }
}

void AddCommandTests::addAndDontAttachVectorsEmptyDirTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile.jpg" << "/another/path/to/some/other/file.jpg";
    vectors << "somefile.eps" << "file.eps";

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, vectors, 0));
    auto result = commandManagerMock.processCommand(addArtworksCommand);
    auto addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    int newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, 2);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(!artItemsModel->getMockArtwork(i)->hasVectorAttached());
    }
}

void AddCommandTests::addAndDontAttachVectorsStartsWithTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile.jpg" << "/another/path/to/some/other/file.jpg";
    vectors << "/to/somefile.eps" << "/path/to/some/other/file.eps";

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, vectors, 0));
    auto result = commandManagerMock.processCommand(addArtworksCommand);
    auto addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    int newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, 2);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(!artItemsModel->getMockArtwork(i)->hasVectorAttached());
    }
}

void AddCommandTests::addAndAttachFromSingleDirectoryTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile1.jpg" << "/path/to/somefile2.jpg" << "/another/path/to/somefile1.jpg" << "/another/path/to/somefile2.jpg";
    vectors << "/path/to/somefile1.eps" << "/path/to/somefile2.eps" << "/another/path/to/somefile1.eps" << "/another/path/to/somefile2.eps";

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, vectors, 0));
    auto result = commandManagerMock.processCommand(addArtworksCommand);
    auto addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    int newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, filenames.length());

    for (int i = 0; i < filenames.length(); ++i) {
        QVERIFY(artItemsModel->getMockArtwork(i)->hasVectorAttached());
    }
}

void AddCommandTests::addSingleDirectoryAndAttachLaterTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile1.jpg" << "/path/to/somefile2.jpg" << "/another/path/to/somefile1.jpg" << "/another/path/to/somefile2.jpg";
    vectors << "/path/to/somefile1.eps" << "/path/to/somefile2.eps" << "/another/path/to/somefile1.eps" << "/another/path/to/somefile2.eps";

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, QStringList(), 0));
    auto result = commandManagerMock.processCommand(addArtworksCommand);
    auto addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    int newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, filenames.length());

    for (int i = 0; i < filenames.length(); ++i) {
        QVERIFY(!artItemsModel->getMockArtwork(i)->hasVectorAttached());
    }

    std::shared_ptr<Commands::AddArtworksCommand> anotherAddArtworksCommand(new Commands::AddArtworksCommand(QStringList(), vectors, 0));
    result = commandManagerMock.processCommand(anotherAddArtworksCommand);
    addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, 0);

    for (int i = 0; i < filenames.length(); ++i) {
        QVERIFY(artItemsModel->getMockArtwork(i)->hasVectorAttached());
    }
}
