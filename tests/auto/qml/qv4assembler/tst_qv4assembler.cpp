/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/qprocess.h>
#include <QtCore/qtemporaryfile.h>
#include <QtQml/qqml.h>
#include <QtQml/qqmlapplicationengine.h>
#include <QtQuickTestUtils/private/qmlutils_p.h>

#include <private/qv4global_p.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class tst_QV4Assembler : public QQmlDataTest
{
    Q_OBJECT

public:
    tst_QV4Assembler();

private slots:
    void initTestCase() override;
    void perfMapFile();
    void functionTable();
    void jitEnabled();
};

tst_QV4Assembler::tst_QV4Assembler()
    : QQmlDataTest(QT_QMLTEST_DATADIR)
{
}

void tst_QV4Assembler::initTestCase()
{
    qputenv("QV4_JIT_CALL_THRESHOLD", "0");
    QQmlDataTest::initTestCase();
}

void tst_QV4Assembler::perfMapFile()
{
#if !defined(Q_OS_LINUX) || defined(Q_OS_ANDROID)
    QSKIP("perf map files are only generated on linux");
#else
    const QString qmljs = QLibraryInfo::path(QLibraryInfo::BinariesPath) + "/qmljs";
    QProcess process;

    QTemporaryFile infile;
    QVERIFY(infile.open());
    infile.write("'use strict'; function foo() { return 42 }; foo();");
    infile.close();

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert("QV4_PROFILE_WRITE_PERF_MAP", "1");
    environment.insert("QV4_JIT_CALL_THRESHOLD", "0");

    process.setProcessEnvironment(environment);
    process.start(qmljs, QStringList({infile.fileName()}));
    QVERIFY(process.waitForStarted());
    const qint64 pid = process.processId();
    QVERIFY(pid != 0);
    QVERIFY(process.waitForFinished());
    QCOMPARE(process.exitCode(), 0);

    QFile file(QString::fromLatin1("/tmp/perf-%1.map").arg(pid));
    QVERIFY(file.exists());
    QVERIFY(file.open(QIODevice::ReadOnly));
    QList<QByteArray> functions;
    while (!file.atEnd()) {
        const QByteArray contents = file.readLine();
        QVERIFY(contents.endsWith('\n'));
        QList<QByteArray> fields = contents.split(' ');
        QCOMPARE(fields.length(), 3);
        bool ok = false;
        const qulonglong address = fields[0].toULongLong(&ok, 16);
        QVERIFY(ok);
        QVERIFY(address > 0);
        const ulong size = fields[1].toULong(&ok, 16);
        QVERIFY(ok);
        QVERIFY(size > 0);
        functions.append(fields[2]);
    }
    QVERIFY(functions.contains("foo\n"));
#endif
}

#ifdef Q_OS_WIN
class Crash : public QObject
{
    Q_OBJECT
public:
    explicit Crash(QObject *parent = nullptr) : QObject(parent) { }
    Q_INVOKABLE static void crash();
};

static bool crashHandlerHit = false;

static LONG WINAPI crashHandler(EXCEPTION_POINTERS*)
{
    crashHandlerHit = true;
    return EXCEPTION_CONTINUE_EXECUTION;
}

void Crash::crash()
{
    SetUnhandledExceptionFilter(crashHandler);
    RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, nullptr);
}
#endif

void tst_QV4Assembler::functionTable()
{
#ifndef Q_OS_WIN
    QSKIP("Function tables only exist on windows.");
#else
    QQmlApplicationEngine engine;
    qmlRegisterType<Crash>("Crash", 1, 0, "Crash");
    engine.load(testFileUrl("crash.qml"));
    QTRY_VERIFY(crashHandlerHit);
#endif
}

void tst_QV4Assembler::jitEnabled()
{
#if defined(Q_OS_IOS) || defined(Q_OS_TVOS)
    /* JIT should be disabled on iOS and tvOS. */
    QVERIFY(!QT_CONFIG(qml_jit));
#elif defined(Q_OS_ANDROID) && defined(Q_PROCESSOR_ARM)
    /* JIT is disabled for Android on ARM/ARM64 before Qt 6.4, see QTBUG-102776. */
    QVERIFY(!QT_CONFIG(qml_jit));
#elif defined(Q_OS_WIN) && defined(Q_PROCESSOR_ARM)
    /* JIT should be disabled Windows on ARM/ARM64 for now. */
    QVERIFY(!QT_CONFIG(qml_jit));
#elif defined(Q_OS_MACOS) && defined(Q_PROCESSOR_ARM)
    /* JIT should be disabled on macOS on ARM/ARM64 for now. */
    QVERIFY(!QT_CONFIG(qml_jit));
#else
    /* JIT should be enabled on all other architectures/OSes tested in CI. */
    QVERIFY(QT_CONFIG(qml_jit));
#endif
}

QTEST_MAIN(tst_QV4Assembler)

#include "tst_qv4assembler.moc"

