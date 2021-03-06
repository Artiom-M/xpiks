#ifndef CORETESTSENVIRONMENT_H
#define CORETESTSENVIRONMENT_H

#include <QDir>
#include <QString>
#include <QStringList>

#include "Common/isystemenvironment.h"

namespace Mocks {
    class CoreTestsEnvironment: public Common::ISystemEnvironment {
        virtual QString root() const override { return QString(); }
        virtual QString path(const QStringList &path) override { return path.join(QDir::separator()); }
        virtual bool ensureDirExists(const QString &) override { return true; }
        virtual bool getIsInMemoryOnly() const override { return true; }
        virtual bool getIsRecoveryMode() const override { return false; }
    };
}

#endif // CORETESTSENVIRONMENT_H
