#ifndef VECTORFILENAMES_TESTS_H
#define VECTORFILENAMES_TESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class VectorFileNamesTests : public QObject
{
    Q_OBJECT
private slots:
    void simpleFilenamesJpgTest();
    void simpleFilenamesTiffTest();
    void filenamesNotReplacedTest();
    void simpleArchivePathTest();
};

#endif // VECTORFILENAMES_TESTS_H
