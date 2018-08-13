#include "removefromuserdictionarytest.h"
#include <QList>
#include <QUrl>
#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"

QString RemoveFromUserDictionaryTest::testName() {
    return QLatin1String("RemoveFromUserDictionaryTest");
}

void RemoveFromUserDictionaryTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

#define CHECK_HAS_ERRORS_EVERYWHERE(basicModel)\
    VERIFY(basicModel->hasDescriptionSpellError(), "Description spell error not detected");\
    VERIFY(basicModel->hasTitleSpellError(), "Title spell error not detected");\
    VERIFY(basicModel->hasKeywordsSpellError(), "Keywords spell error not detected")

#define CHECK_HAS_NO_ERRORS_ANYWHERE(basicModel)\
    VERIFY(!basicModel->hasDescriptionSpellError(), "After adding word. Description spell error is still present");\
    VERIFY(!basicModel->hasTitleSpellError(), "After adding word. Title spell error is still present");\
    VERIFY(!basicModel->hasKeywordsSpellError(), "After adding word. Keywords spell error is still present");


int RemoveFromUserDictionaryTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    Artworks::ArtworkMetadata *artwork = m_TestsApp.getArtwork(0);

    // wait for after-add spellchecking
    QThread::sleep(1);

    SignalWaiter spellingWaiter;
    m_TestsApp.connectWaiterForSpellcheck(spellingWaiter);

    auto *basicKeywordsModel = artwork->getBasicModel();

    QString wrongWord = "abbreviatioe";
    artwork->setDescription(artwork->getDescription() + " " + wrongWord);
    artwork->setTitle(artwork->getTitle() + " " + wrongWord);
    artwork->appendKeyword("correct part " + wrongWord);
    artwork->setIsSelected(true);

    VERIFY(spellingWaiter.wait(5), "Timeout for waiting for spellcheck results");

    // wait for finding suggestions
    QThread::sleep(1);

    CHECK_HAS_ERRORS_EVERYWHERE(basicKeywordsModel);

    auto &userDictionary = m_TestsApp.getUserDictionary();
    userDictionary.addWord(wrongWord);

    QCoreApplication::processEvents(QEventLoop::AllEvents);

    // wait add user word to finish
    VERIFY(spellingWaiter.wait(5), "Timeout for waiting for spellcheck results");

    sleepWaitUntil(5, [=]() {
        return !basicKeywordsModel->hasDescriptionSpellError() &&
                !basicKeywordsModel->hasTitleSpellError() &&
                !basicKeywordsModel->hasKeywordsSpellError();
    });

    int userDictWords = userDictionary.getWordsCount();

    VERIFY(userDictWords == 1, "Wrong number of words in user dictionary");
    CHECK_HAS_NO_ERRORS_ANYWHERE(basicKeywordsModel);

    // now clean user dict
    userDictionary.clear();

    // wait clear user dict to finish
    QCoreApplication::processEvents(QEventLoop::AllEvents);

    // wait clear user word to finish
    VERIFY(spellingWaiter.wait(5), "Timeout for waiting for spellcheck results");

    sleepWaitUntil(5, [=]() {
        return basicKeywordsModel->hasDescriptionSpellError() &&
                basicKeywordsModel->hasTitleSpellError() &&
                basicKeywordsModel->hasKeywordsSpellError();
    });

    userDictWords = userDictionary.getWordsCount();

    VERIFY(userDictWords == 0, "Wrong number of words in user dictionary");    
    CHECK_HAS_ERRORS_EVERYWHERE(basicKeywordsModel);

    return 0;
}
