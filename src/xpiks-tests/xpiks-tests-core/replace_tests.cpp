#include "replace_tests.h"
#include <QString>
#include <QtAlgorithms>
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/coretestsenvironment.h"
#include "Mocks/artworksrepositorymock.h"
#include "../../xpiks-qt/Commands/findandreplacecommand.h"
#include "../../xpiks-qt/Models/filteredartitemsproxymodel.h"
#include "../../xpiks-qt/Models/artworksrepository.h"
#include "../../xpiks-qt/Models/previewartworkelement.h"
#include "../../xpiks-qt/Common/flags.h"

#define DECLARE_MODELS_AND_GENERATE(count) \
    Mocks::CoreTestsEnvironment environment; \
    Mocks::CommandManagerMock commandManagerMock; \
    Mocks::ArtworksListModelMock ArtworksListModelMock; \
    Mocks::ArtworksRepositoryMock artworksRepository(environment); \
    Models::FilteredArtItemsProxyModel filteredItemsModel; \
    commandManagerMock.InjectDependency(&artworksRepository); \
    commandManagerMock.InjectDependency(&ArtworksListModelMock); \
    filteredItemsModel.setSourceModel(&ArtworksListModelMock); \
    commandManagerMock.InjectDependency(&filteredItemsModel); \
    commandManagerMock.generateAndAddArtworks(count);

void ReplaceTests::replaceTrivialTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "Replace";
    QString replaceTo = "Replaced";
    QString initString = "ReplaceMe";
    QString finalString = "ReplacedMe";

    auto flags = Common::SearchFlags::CaseSensitive |
                Common::SearchFlags::Description |
                Common::SearchFlags::Title |
                Common::SearchFlags::Keywords;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        metadata->set(initString, initString, QStringList() << initString);
    }

    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(
                new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags));
    auto result = commandManagerMock.processCommand(replaceCommand);

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        QCOMPARE(metadata->getDescription(), finalString);
        QCOMPARE(metadata->getTitle(), finalString);
        QCOMPARE(metadata->getKeywords()[0], finalString);
        QVERIFY(metadata->isModified());
    }
}

void ReplaceTests::noReplaceTrivialTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "Noreplace";
    QString replaceTo = "Replaced";
    QString initString = "ReplaceMe";
    QString finalString = "ReplaceMe";

    auto flags = Common::SearchFlags::CaseSensitive |
                Common::SearchFlags::Description |
                Common::SearchFlags::Title |
                Common::SearchFlags::Keywords;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        metadata->set(initString, initString, QStringList() << initString);
    }

    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(
                new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags));
    auto result = commandManagerMock.processCommand(replaceCommand);

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        QCOMPARE(metadata->getDescription(), finalString);
        QCOMPARE(metadata->getTitle(), finalString);
        QCOMPARE(metadata->getKeywords()[0], finalString);
        QVERIFY(!metadata->isModified());
    }
}

void ReplaceTests::caseSensitiveTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "rePLace";
    QString replaceTo = "Replaced";
    QString initString = "ReplaceMe";
    QString finalString = "ReplacedMe";

    auto flags = Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        metadata->set(initString, initString, QStringList() << initString);
    }

    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(
                new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags));
    auto result = commandManagerMock.processCommand(replaceCommand);

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        QCOMPARE(metadata->getDescription(), finalString);
        QCOMPARE(metadata->getTitle(), finalString);
        QCOMPARE(metadata->getKeywords()[0], finalString);
        QVERIFY(metadata->isModified());
    }
}

void ReplaceTests::replaceTitleTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "Replace";
    QString replaceTo = "Replaced";
    QString initString = "ReplaceMe";
    QString finalString = "ReplacedMe";

    auto flags = Common::SearchFlags::CaseSensitive | Common::SearchFlags::Title;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        metadata->set(initString, initString, QStringList() << initString);
    }

    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(
                new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags));
    auto result = commandManagerMock.processCommand(replaceCommand);

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        QCOMPARE(metadata->getDescription(), initString);
        QCOMPARE(metadata->getTitle(), finalString);
        QCOMPARE(metadata->getKeywords()[0], initString);
        QVERIFY(metadata->isModified());
    }
}

void ReplaceTests::replaceKeywordsTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "keywordOld";
    QString replaceTo = "keywordNew";
    QString replaceToLower = replaceTo.toLower();

    auto flags = Common::SearchFlags::CaseSensitive |
            Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        metadata->set(QString("title"), QString("description"),
                             QStringList() << replaceToLower << "dummyKey" << replaceFrom);
    }

    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(
                new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags));
    auto result = commandManagerMock.processCommand(replaceCommand);

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        QCOMPARE(metadata->getDescription(), QString("description"));
        QCOMPARE(metadata->getTitle(), QString("title"));

        QStringList test = metadata->getKeywords();
        QStringList gold;

        gold << replaceToLower << "dummyKey";
        qSort(gold.begin(), gold.end());
        qSort(test.begin(), test.end());

        QCOMPARE(gold, test);
        QVERIFY(metadata->isModified());
    }
}

void ReplaceTests::replaceToSpaceTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = " vector ";
    QString replaceTo = " ";

    auto flags = Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        metadata->set(QString("A vector can be found here"), QString("And here"),
                             QStringList());
    }

    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(
                new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags));
    auto result = commandManagerMock.processCommand(replaceCommand);

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        QCOMPARE(metadata->getDescription(), QString("And here"));
        QCOMPARE(metadata->getTitle(), QString("A can be found here"));
        QVERIFY(metadata->isModified());
    }
}

void ReplaceTests::replaceToNothingTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = " vector ";
    QString replaceTo = "";

    auto flags = Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        metadata->set(QString("A vector can be found here"), QString("And here vector  as well"),
                             QStringList());
    }

    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(
                new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags));
    auto result = commandManagerMock.processCommand(replaceCommand);

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        QCOMPARE(metadata->getDescription(), QString("And here as well"));
        QCOMPARE(metadata->getTitle(), QString("Acan be found here"));
        QVERIFY(metadata->isModified());
    }
}

void ReplaceTests::spacesReplaceCaseSensitiveTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = " vector ";
    QString replaceTo = " ";

    auto flags = Common::SearchFlags::CaseSensitive |
            Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        metadata->set(QString("A Vector can be found here"), QString("And vector here"),
                             QStringList());
    }

    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(
                new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags));
    auto result = commandManagerMock.processCommand(replaceCommand);

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        QCOMPARE(metadata->getDescription(), QString("And here"));
        QCOMPARE(metadata->getTitle(), QString("A Vector can be found here"));
        QVERIFY(metadata->isModified());
    }
}

void ReplaceTests::spaceReplaceCaseSensitiveNoReplaceTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = " Vector ";
    QString replaceTo = " ";

    auto flags = Common::SearchFlags::CaseSensitive |
            Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        metadata->set(QString("A vector can be found here"), QString("And vector here"),
                             QStringList());
    }

    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(
                new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags));
    auto result = commandManagerMock.processCommand(replaceCommand);

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        QCOMPARE(metadata->getDescription(), QString("And vector here"));
        QCOMPARE(metadata->getTitle(), QString("A vector can be found here"));
        QVERIFY(!metadata->isModified());
    }
}

void ReplaceTests::replaceSpacesToWordsTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "  ";
    QString replaceTo = "word";

    auto flags = Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        metadata->set(QString("A   here"), QString("And    here"),
                             QStringList());
    }

    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(
                new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags));
    auto result = commandManagerMock.processCommand(replaceCommand);

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        QCOMPARE(metadata->getDescription(), QString("Andwordwordhere"));
        QCOMPARE(metadata->getTitle(), QString("Aword here"));
    }
}

void ReplaceTests::replaceSpacesToSpacesTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "  ";
    QString replaceTo = " ";

    auto flags = Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        metadata->set(QString("A   here"), QString("And    here"),
                             QStringList());
    }

    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(
                new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags));
    auto result = commandManagerMock.processCommand(replaceCommand);

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        QCOMPARE(metadata->getDescription(), QString("And  here"));
        QCOMPARE(metadata->getTitle(), QString("A  here"));
    }
}

void ReplaceTests::replaceKeywordsToEmptyTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "vector";
    QString replaceTo = " ";

    auto flags = Common::SearchFlags::CaseSensitive |
            Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        metadata->set(QString("A Vector can be found here"), QString("And vector here"),
                             QStringList() << "a vector here" << " vector ");
    }

    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(
                new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags));
    auto result = commandManagerMock.processCommand(replaceCommand);

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *metadata = ArtworksListModelMock.getMockArtwork(i);
        QCOMPARE(metadata->getDescription(), QString("And   here"));
        QCOMPARE(metadata->getTitle(), QString("A Vector can be found here"));

        QStringList test = metadata->getKeywords();
        QStringList gold;
        gold << "a here";

        QCOMPARE(gold, test);
        QVERIFY(metadata->isModified());
    }
}

