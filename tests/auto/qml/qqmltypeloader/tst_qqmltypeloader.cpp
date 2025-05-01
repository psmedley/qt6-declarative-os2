// Copyright (C) 2016 Canonical Limited and/or its subsidiary(-ies).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlfile.h>
#include <QtQml/qqmlnetworkaccessmanagerfactory.h>
#include <QtQuick/qquickview.h>
#include <QtQuick/qquickitem.h>
#if QT_CONFIG(process)
#include <QtCore/qprocess.h>
#endif
#include <QtQml/private/qqmlcomponent_p.h>
#include <QtQml/private/qqmlengine_p.h>
#include <QtQml/private/qqmlirbuilder_p.h>
#include <QtQml/private/qqmlirloader_p.h>
#include <QtQml/private/qqmltypedata_p.h>
#include <QtQml/private/qqmltypeloader_p.h>
#include <QtQuickTestUtils/private/testhttpserver_p.h>
#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QQmlComponent>

class tst_QQMLTypeLoader : public QQmlDataTest
{
    Q_OBJECT

public:
    tst_QQMLTypeLoader();

private slots:
    void testLoadComplete();
    void loadComponentSynchronously();
    void trimCache();
    void trimCache2();
    void trimCache3();
    void keepSingleton();
    void keepRegistrations();
    void importAndDestroy();
    void intercept();
    void redirect();
    void qmlSingletonWithinModule();
    void multiSingletonModule();
    void multiSingletonModuleNoWarning();
    void implicitComponentModule();
    void customDiskCachePath();
    void qrcRootPathUrl();
    void implicitImport();
    void compositeSingletonCycle();
    void declarativeCppType();
    void circularDependency();
    void declarativeCppAndQmlDir();
    void signalHandlersAreCompatible();
    void loadTypeOnShutdown();
    void floodTypeLoaderEventQueue();
    void retainQmlTypeAcrossEngines();

private:
    void checkSingleton(const QString & dataDirectory);
};

tst_QQMLTypeLoader::tst_QQMLTypeLoader()
    : QQmlDataTest(QT_QMLTEST_DATADIR)
{
}

void tst_QQMLTypeLoader::testLoadComplete()
{
#ifdef Q_OS_ANDROID
    QSKIP("Loading dynamic plugins does not work on Android");
#endif
    std::unique_ptr<QQuickView> window = std::make_unique<QQuickView>();
    window->engine()->addImportPath(QT_TESTCASE_BUILDDIR);
    qDebug() << window->engine()->importPathList();
    window->setGeometry(0,0,240,320);
    window->setSource(testFileUrl("test_load_complete.qml"));
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window.get()));

    QObject *rootObject = window->rootObject();
    QTRY_VERIFY(rootObject != nullptr);
    QTRY_COMPARE(rootObject->property("created").toInt(), 2);
    QTRY_COMPARE(rootObject->property("loaded").toInt(), 2);
}

void tst_QQMLTypeLoader::loadComponentSynchronously()
{
    QQmlEngine engine;
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(
                             QLatin1String(".*nonprotocol::1:1: QtObject is not a type.*")));
    QQmlComponent component(&engine, testFileUrl("load_synchronous.qml"));
    QScopedPointer<QObject> o(component.create());
    QVERIFY(o);
}

void tst_QQMLTypeLoader::trimCache()
{
    QQmlEngine engine;
    QQmlTypeLoader &loader = QQmlEnginePrivate::get(&engine)->typeLoader;
    QVector<QQmlTypeData *> releaseLater;
    QVector<QV4::CompiledData::CompilationUnit *> releaseCompilationUnitLater;
    for (int i = 0; i < 256; ++i) {
        QUrl url = testFileUrl("trim_cache.qml");
        url.setQuery(QString::number(i));

        QQmlTypeData *data = loader.getType(url).take();

        // Backup source code should be dropped right after loading, even without cache trimming.
        QVERIFY(!data->backupSourceCode().isValid());

        // Run an event loop to receive the callback that release()es.
        QTRY_COMPARE(data->count(), 2);

        // keep references to some of them so that they aren't trimmed. References to either the
        // QQmlTypeData or its compiledData() should prevent the trimming.
        if (i % 10 == 0) {
            // keep ref on data, don't add ref on data->compiledData()
            releaseLater.append(data);
        } else if (i % 5 == 0) {
            data->compilationUnit()->addref();
            releaseCompilationUnitLater.append(data->compilationUnit());
            data->release();
        } else {
            data->release();
        }
    }

    for (int i = 0; i < 256; ++i) {
        QUrl url = testFileUrl("trim_cache.qml");
        url.setQuery(QString::number(i));
        if (i % 5 == 0)
            QVERIFY(loader.isTypeLoaded(url));
        else if (i < 128)
            QVERIFY(!loader.isTypeLoaded(url));
        // The cache is free to keep the others.
    }

    for (auto *data : std::as_const(releaseCompilationUnitLater))
        data->release();

    for (auto *data : std::as_const(releaseLater))
        data->release();
}

void tst_QQMLTypeLoader::trimCache2()
{
    QScopedPointer<QQuickView> window(new QQuickView());
    window->setSource(testFileUrl("trim_cache2.qml"));
    QQmlTypeLoader &loader = QQmlEnginePrivate::get(window->engine())->typeLoader;
    // in theory if gc has already run this could be false
    // QCOMPARE(loader.isTypeLoaded(testFileUrl("MyComponent2.qml")), true);
    window->engine()->collectGarbage();
    QTest::qWait(1);    // force event loop
    window->engine()->trimComponentCache();
    QCOMPARE(loader.isTypeLoaded(testFileUrl("MyComponent2.qml")), false);
}

// test trimming the cache of an item that contains sub-items created via incubation
void tst_QQMLTypeLoader::trimCache3()
{
    QScopedPointer<QQuickView> window(new QQuickView());
    window->setSource(testFileUrl("trim_cache3.qml"));
    QQmlTypeLoader &loader = QQmlEnginePrivate::get(window->engine())->typeLoader;
    QCOMPARE(loader.isTypeLoaded(testFileUrl("ComponentWithIncubator.qml")), true);

    QQmlProperty::write(window->rootObject(), "source", QString());

    // handle our deleteLater and cleanup
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
    window->engine()->collectGarbage();

    window->engine()->trimComponentCache();

    QCOMPARE(loader.isTypeLoaded(testFileUrl("ComponentWithIncubator.qml")), false);
}

void tst_QQMLTypeLoader::checkSingleton(const QString &dataDirectory)
{
    QQmlEngine engine;
    engine.addImportPath(dataDirectory);
    QQmlComponent component(&engine);
    component.setData("import ClusterDemo 1.0\n"
                      "import QtQuick 2.6\n"
                      "import \"..\"\n"
                      "Item { property int t: ValueSource.something }",
                      testFileUrl("abc/Xyz.qml"));
    QVERIFY2(component.status() == QQmlComponent::Ready, qPrintable(component.errorString()));
    QScopedPointer<QObject> o(component.create());
    QVERIFY(o.data());
    QCOMPARE(o->property("t").toInt(), 10);
}

void tst_QQMLTypeLoader::keepSingleton()
{
    qmlRegisterSingletonType(testFileUrl("ValueSource.qml"), "ClusterDemo", 1, 0, "ValueSource");
    checkSingleton(dataDirectory());
    QQmlMetaType::freeUnusedTypesAndCaches();
    checkSingleton(dataDirectory());
}

class TestObject : public QObject
{
    Q_OBJECT
public:
    TestObject(QObject *parent = nullptr) : QObject(parent) {}
};

QML_DECLARE_TYPE(TestObject)

static void verifyTypes(bool shouldHaveTestObject, bool shouldHaveFast)
{
    bool hasTestObject = false;
    bool hasFast = false;
    for (const QQmlType &type : QQmlMetaType::qmlAllTypes()) {
        if (type.elementName() == QLatin1String("Fast"))
            hasFast = true;
        else if (type.elementName() == QLatin1String("TestObject"))
            hasTestObject = true;
    }
    QCOMPARE(hasTestObject, shouldHaveTestObject);
    QCOMPARE(hasFast, shouldHaveFast);
}

void tst_QQMLTypeLoader::keepRegistrations()
{
    verifyTypes(false, false);
    qmlRegisterType<TestObject>("Test", 1, 0, "TestObject");
    verifyTypes(true, false);

    {
        QQmlEngine engine;
        engine.addImportPath(dataDirectory());
        QQmlComponent component(&engine);
        component.setData("import Fast 1.0\nFast {}", QUrl());
        QVERIFY2(component.errorString().isEmpty(), component.errorString().toUtf8().constData());
        QCOMPARE(component.status(), QQmlComponent::Ready);
        QScopedPointer<QObject> o(component.create());
        QVERIFY(o.data());
        verifyTypes(true, true);
    }

    verifyTypes(true, false); // Fast is gone again, even though an event was still scheduled.
    QQmlMetaType::freeUnusedTypesAndCaches();
    verifyTypes(true, false); // qmlRegisterType creates an undeletable type.
}

class NetworkReply : public QNetworkReply
{
public:
    NetworkReply()
    {
        open(QIODevice::ReadOnly);
    }

    void setData(const QByteArray &data)
    {
        if (isFinished())
            return;
        m_buffer = data;
        emit readyRead();
        setFinished(true);
        emit finished();
    }

    void fail()
    {
        if (isFinished())
            return;
        m_buffer.clear();
        setError(ContentNotFoundError, "content not found");
        emit errorOccurred(ContentNotFoundError);
        setFinished(true);
        emit finished();
    }

    qint64 bytesAvailable() const override
    {
        return m_buffer.size();
    }

    qint64 readData(char *data, qint64 maxlen) override
    {
        if (m_buffer.size() < maxlen)
            maxlen = m_buffer.size();
        std::memcpy(data, m_buffer.data(), maxlen);
        m_buffer.remove(0, maxlen);
        return maxlen;
    }

    void abort() override
    {
        if (isFinished())
            return;
        m_buffer.clear();
        setFinished(true);
        emit finished();
    }

private:
    QByteArray m_buffer;
};

class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:

    NetworkAccessManager(QObject *parent) : QNetworkAccessManager(parent)
    {
    }

    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request,
                                 QIODevice *outgoingData) override
    {
        QUrl url = request.url();
        QString scheme = url.scheme();
        if (op != GetOperation || !scheme.endsWith("+debug"))
            return QNetworkAccessManager::createRequest(op, request, outgoingData);

        scheme.chop(sizeof("+debug") - 1);
        url.setScheme(scheme);

        NetworkReply *reply = new NetworkReply;
        QString filename = QQmlFile::urlToLocalFileOrQrc(url);
        QTimer::singleShot(QRandomGenerator::global()->bounded(20), reply,
                           [this, reply, filename]() {
            if (filename.isEmpty()) {
                reply->fail();
            } else {
                QFile file(filename);
                if (file.open(QIODevice::ReadOnly)) {
                    emit loaded(filename);
                    reply->setData(transformQmldir(filename, file.readAll()));
                } else
                    reply->fail();
            }
        });
        return reply;
    }

    QByteArray transformQmldir(const QString &filename, const QByteArray &content)
    {
        if (!filename.endsWith("/qmldir"))
            return content;

        // Make qmldir plugin paths absolute, so that we don't try to load them over the network
        QByteArray result;
        QByteArray path = filename.toUtf8();
        path.chop(sizeof("qmldir") - 1);
        for (QByteArray line : content.split('\n')) {
            if (line.isEmpty())
                continue;
            QList<QByteArray> segments = line.split(' ');
            if (segments.startsWith("optional")) {
                result.append("optional ");
                segments.removeFirst();
            }
            if (segments.startsWith("plugin")) {
                if (segments.size() == 2) {
                    segments.append(path);
                } else if (segments.size() == 3) {
                    if (!segments[2].startsWith('/'))
                        segments[2] = path + segments[2];
                } else {
                    // Invalid plugin declaration. Ignore
                }
                result.append(segments.join(' '));
            } else {
                result.append(line);
            }
            result.append('\n');
        }
        return result;
    }

signals:
    void loaded(const QString &filename);
};

class NetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
public:
    QStringList loadedFiles;

    QNetworkAccessManager *create(QObject *parent) override
    {
        NetworkAccessManager *manager = new NetworkAccessManager(parent);
        QObject::connect(manager, &NetworkAccessManager::loaded, [this](const QString &filename) {
            loadedFiles.append(filename);
        });
        return manager;
    }
};

class ManualRedirectNetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
public:
    QStringList loadedFiles;

    QNetworkAccessManager *create(QObject *parent) override
    {
        NetworkAccessManager *manager = new NetworkAccessManager(parent);
        manager->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);
        return manager;
    }
};

class UrlInterceptor : public QQmlAbstractUrlInterceptor
{
public:
    QUrl intercept(const QUrl &path, DataType type) override
    {
        Q_UNUSED(type);
        if (!QQmlFile::isLocalFile(path))
            return path;

        QUrl result = path;
        QString scheme = result.scheme();
        if (!scheme.endsWith("+debug"))
            result.setScheme(scheme + "+debug");
        return result;
    }
};

void tst_QQMLTypeLoader::importAndDestroy()
{
#if defined Q_OS_ANDROID || defined Q_OS_IOS
    QSKIP("Data directory is not in the host file system on Android and iOS");
#endif
    qmlClearTypeRegistrations();

    QQmlEngine engine;
    NetworkAccessManagerFactory factory;
    engine.setNetworkAccessManagerFactory(&factory);
    QQmlComponent component(&engine);

    // We redirect the import through the network access manager to make it asynchronous.
    // Otherwise the type loader will just directly call back into the main thread and we
    // won't get a chance to do mischief before initializeEngine gets called for the "Slow"
    // module. Note that the "Slow" module needs to be loaded from a "local" URL since plugins
    // can only be loaded locally.

    // Detour through testFileUrl to get the path right on windows ('C:' and things like that)
    QUrl url = testFileUrl("SlowImporter");
    url.setScheme(url.scheme() + QLatin1String("+debug"));

    component.setData(QString::fromLatin1(R"(
        import '%1'
        A {}
    )").arg(url.toString()).toUtf8(), QUrl());

    while (!QQmlMetaType::qmlType(
                    QStringLiteral("SlowStuff"), QStringLiteral("Slow"), QTypeRevision())
                    .isValid()) {
        // busy wait for type to be registered
        QVERIFY2(!component.isError(), qPrintable(component.errorString()));
    }

    // Now the type loader thread is likely waiting for the main thread to process the
    // initializeEngine callback. We destroy the engine here to trigger the situation where the main
    // thread needs to wake the type loader thread one more time to process the isShutdown flag.
    // If it fails to do so, the type loader thread waits indefinitely for the main thread and the
    // engine dtor in turn waits indefinitely for the type loader thread to terminate.

    // The point of this test is that it _should not_ deadlock here.
}

void tst_QQMLTypeLoader::intercept()
{
#ifdef Q_OS_ANDROID
    QSKIP("Loading dynamic plugins does not work on Android");
#endif
    qmlClearTypeRegistrations();

    QQmlEngine engine;
    engine.addImportPath(dataDirectory());
    engine.addImportPath(QT_TESTCASE_BUILDDIR);

    UrlInterceptor interceptor;
    NetworkAccessManagerFactory factory;

    engine.addUrlInterceptor(&interceptor);
    engine.setNetworkAccessManagerFactory(&factory);

    QQmlComponent component(&engine, testFileUrl("test_intercept.qml"));

    QVERIFY(component.status() != QQmlComponent::Ready);
    QTRY_VERIFY2(component.status() == QQmlComponent::Ready,
                 component.errorString().toUtf8().constData());

    QScopedPointer<QObject> o(component.create());
    QVERIFY(o.data());

    QTRY_COMPARE(o->property("created").toInt(), 2);
    QTRY_COMPARE(o->property("loaded").toInt(), 2);

    QVERIFY(factory.loadedFiles.size() >= 6);
    QVERIFY(factory.loadedFiles.contains(dataDirectory() + "/test_intercept.qml"));
    QVERIFY(factory.loadedFiles.contains(dataDirectory() + "/Intercept.qml"));
    QVERIFY(factory.loadedFiles.contains(dataDirectory() + "/Fast/qmldir"));
    QVERIFY(factory.loadedFiles.contains(dataDirectory() + "/Fast/Fast.qml"));
    QVERIFY(factory.loadedFiles.contains(dataDirectory() + "/GenericView.qml"));
    QVERIFY(factory.loadedFiles.contains(QLatin1String(QT_TESTCASE_BUILDDIR) + "/Slow/qmldir"));
}

void tst_QQMLTypeLoader::redirect()
{
    TestHTTPServer server;
    QVERIFY2(server.listen(), qPrintable(server.errorString()));
    QVERIFY(server.serveDirectory(dataDirectory()));
    server.addRedirect("Base.qml", server.urlString("/redirected/Redirected.qml"));

    ManualRedirectNetworkAccessManagerFactory factory;
    QQmlEngine engine;
    engine.setNetworkAccessManagerFactory(&factory);
    QQmlComponent component(&engine);
    component.loadUrl(server.urlString("/Load.qml"), QQmlComponent::Asynchronous);
    QTRY_VERIFY2(component.isReady(), qPrintable(component.errorString()));

    QScopedPointer<QObject> object {component.create()};
    QTRY_COMPARE(object->property("xy").toInt(), 323232);
}

void tst_QQMLTypeLoader::qmlSingletonWithinModule()
{
    qmlClearTypeRegistrations();
    QQmlEngine engine;
    qmlRegisterSingletonType(testFileUrl("Singleton.qml"), "modulewithsingleton", 1, 0, "Singleton");

    QQmlComponent component(&engine, testFileUrl("singletonuser.qml"));
    QCOMPARE(component.status(), QQmlComponent::Ready);
    QScopedPointer<QObject> obj(component.create());
    QVERIFY(!obj.isNull());
    QVERIFY(obj->property("ok").toBool());
}

static void checkCleanCacheLoad(const QString &testCase)
{
#if QT_CONFIG(process)
    const char *skipKey = "QT_TST_QQMLTYPELOADER_SKIP_MISMATCH";
    if (qEnvironmentVariableIsSet(skipKey))
        return;
    for (int i = 0; i < 5; ++i) {
        QProcess child;
        child.setProgram(QCoreApplication::applicationFilePath());
        child.setArguments(QStringList(testCase));
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QLatin1String("QT_LOGGING_RULES"), QLatin1String("qt.qml.diskcache.debug=true"));
        env.insert(QLatin1String(skipKey), QLatin1String("1"));
        child.setProcessEnvironment(env);
        child.start();
        QVERIFY(child.waitForFinished());
        QCOMPARE(child.exitCode(), 0);
        QVERIFY(!child.readAllStandardOutput().contains("Checksum mismatch for cached version"));
        QVERIFY(!child.readAllStandardError().contains("Checksum mismatch for cached version"));
    }
#else
    Q_UNUSED(testCase);
#endif
}

void tst_QQMLTypeLoader::multiSingletonModule()
{
#ifdef Q_OS_ANDROID
    QSKIP("Android seems to have problems with QProcess");
#endif
    qmlClearTypeRegistrations();
    QQmlEngine engine;
    engine.addImportPath(testFile("imports"));

    qmlRegisterSingletonType(testFileUrl("CppRegisteredSingleton1.qml"), "cppsingletonmodule",
                             1, 0, "CppRegisteredSingleton1");
    qmlRegisterSingletonType(testFileUrl("CppRegisteredSingleton2.qml"), "cppsingletonmodule",
                             1, 0, "CppRegisteredSingleton2");

    QQmlComponent component(&engine, testFileUrl("multisingletonuser.qml"));
    QCOMPARE(component.status(), QQmlComponent::Ready);
    QScopedPointer<QObject> obj(component.create());
    QVERIFY(!obj.isNull());
    QVERIFY(obj->property("ok").toBool());

    checkCleanCacheLoad(QLatin1String("multiSingletonModule"));
}

void tst_QQMLTypeLoader::multiSingletonModuleNoWarning()
{
    // Should not warn about a "cyclic" dependency between the singletons
    QTest::failOnWarning(QRegularExpression(".*"));

    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("imports/multisingletonmodule/a.qml"));
    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    QScopedPointer<QObject> o(component.create());
    QVERIFY(!o.isNull());
}

void tst_QQMLTypeLoader::implicitComponentModule()
{
#ifdef Q_OS_ANDROID
    QSKIP("Android seems to have problems with QProcess");
#endif
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("implicitcomponent.qml"));
    QCOMPARE(component.status(), QQmlComponent::Ready);
    QScopedPointer<QObject> obj(component.create());
    QVERIFY(!obj.isNull());

    checkCleanCacheLoad(QLatin1String("implicitComponentModule"));
}

void tst_QQMLTypeLoader::customDiskCachePath()
{
#ifdef Q_OS_ANDROID
    QSKIP("Android seems to have problems with QProcess");
#endif

#if QT_CONFIG(process)
    const char *skipKey = "QT_TST_QQMLTYPELOADER_SKIP_MISMATCH";
    if (qEnvironmentVariableIsSet(skipKey)) {
        QQmlEngine engine;
        QQmlComponent component(&engine, testFileUrl("Base.qml"));
        QCOMPARE(component.status(), QQmlComponent::Ready);
        QScopedPointer<QObject> obj(component.create());
        QVERIFY(!obj.isNull());
        return;
    }

    QTemporaryDir dir;
    QProcess child;
    child.setProgram(QCoreApplication::applicationFilePath());
    child.setArguments(QStringList(QLatin1String("customDiskCachePath")));
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QLatin1String(skipKey), QLatin1String("1"));
    env.insert(QLatin1String("QML_DISK_CACHE_PATH"), dir.path());
    child.setProcessEnvironment(env);
    child.start();
    QVERIFY(child.waitForFinished());
    QCOMPARE(child.exitCode(), 0);
    QDir cacheDir(dir.path());
    QVERIFY(!cacheDir.isEmpty());
#endif
}

void tst_QQMLTypeLoader::qrcRootPathUrl()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qrcRootPath.qml"));
    QCOMPARE(component.status(), QQmlComponent::Ready);
}

void tst_QQMLTypeLoader::implicitImport()
{
    QQmlEngine engine;
    engine.addImportPath(testFile("imports"));
    {
        QQmlComponent component(&engine, testFileUrl("implicitimporttest.qml"));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        QScopedPointer<QObject> obj(component.create());
        QVERIFY(!obj.isNull());
    }
    {
        QQmlComponent component(&engine, testFileUrl("implicitautoimporttest.qml"));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        QScopedPointer<QObject> obj(component.create());
        QVERIFY(!obj.isNull());
    }
    {
        QQmlComponent component(&engine, testFileUrl("implicitversionedimporttest.qml"));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        QScopedPointer<QObject> obj(component.create());
        QVERIFY(!obj.isNull());
    }

}

void tst_QQMLTypeLoader::compositeSingletonCycle()
{
    TestHTTPServer server;
    QVERIFY2(server.listen(), qPrintable(server.errorString()));
    QVERIFY(server.serveDirectory(dataDirectory()));

    QQmlEngine engine;
    QQmlComponent component(&engine);
    engine.addImportPath(server.baseUrl().toString());
    component.loadUrl(server.urlString("Com/Orga/Handlers/Handler.qml"), QQmlComponent::Asynchronous);
    QTRY_VERIFY2(component.isReady(), qPrintable(component.errorString()));

    QScopedPointer<QObject> object {component.create()};
    QVERIFY(object);
    QCOMPARE(qvariant_cast<QColor>(object->property("color")), QColorConstants::Black);
}

void tst_QQMLTypeLoader::declarativeCppType()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("declarativeCppType.qml"));
    QCOMPARE(component.status(), QQmlComponent::Ready);
    QScopedPointer<QObject> obj(component.create());
    QVERIFY(!obj.isNull());
}

void tst_QQMLTypeLoader::circularDependency()
{
    QQmlEngine engine;
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Cyclic dependency detected between (.*) and (.*)"));
    QQmlComponent component(&engine, testFileUrl("CircularDependency.qml"));
    QCOMPARE(component.status(), QQmlComponent::Null);
}

void tst_QQMLTypeLoader::declarativeCppAndQmlDir()
{
    QQmlEngine engine;
    engine.addImportPath("qrc:/");
    QQmlComponent component(&engine, testFileUrl("cppAndQmlDir.qml"));
    QVERIFY2(!component.isError(), qPrintable(component.errorString()));
    QScopedPointer<QObject> root(component.create());
    QCOMPARE(root->objectName(), "Singleton");
}

static void getCompilationUnitAndRuntimeInfo(QQmlRefPointer<QV4::ExecutableCompilationUnit> &unit,
                                             QList<int> &runtimeFunctionIndices, const QUrl &url,
                                             QQmlEngine *engine)
{
    QQmlTypeLoader &loader = QQmlEnginePrivate::get(engine)->typeLoader;
    auto typeData = loader.getType(url);
    QVERIFY(typeData);
    QVERIFY(!typeData->backupSourceCode().isValid());

    if (typeData->isError()) {
        const auto errors = typeData->errors();
        for (const QQmlError &e : errors)
            qDebug().noquote() << e.toString();
        QVERIFY(!typeData->isError()); // this returns
    }

    unit = engine->handle()->executableCompilationUnit(typeData->compilationUnit());
    QVERIFY(unit);

    // the QmlIR::Document is deleted once loader.getType() is complete, so
    // restore it
    const QString &urlString = url.toString();
    QmlIR::Document restoredIrDocument(urlString, urlString, false);
    QQmlIRLoader irLoader(unit->unitData(), &restoredIrDocument);
    irLoader.load();
    QCOMPARE(restoredIrDocument.objects.size(), 1);

    const QmlIR::Object *irRoot = restoredIrDocument.objects.at(0);
    runtimeFunctionIndices = QList<int>(irRoot->runtimeFunctionIndices.begin(),
                                        irRoot->runtimeFunctionIndices.end());
}

void tst_QQMLTypeLoader::signalHandlersAreCompatible()
{
    QQmlEngine engine;

    QQmlRefPointer<QV4::ExecutableCompilationUnit> unitFromCachegen;
    QList<int> runtimeFunctionIndicesFromCachegen;
    getCompilationUnitAndRuntimeInfo(unitFromCachegen, runtimeFunctionIndicesFromCachegen,
                                     // use qmlcachegen version
                                     QUrl("qrc:/data/compilercompatibility/signalHandlers.qml"),
                                     &engine);
    if (QTest::currentTestFailed())
        return;

    QQmlRefPointer<QV4::ExecutableCompilationUnit> unitFromTypeCompiler;
    QList<int> runtimeFunctionIndicesFromTypeCompiler;
    getCompilationUnitAndRuntimeInfo(unitFromTypeCompiler, runtimeFunctionIndicesFromTypeCompiler,
                                     // use qqmltypecompiler version
                                     testFileUrl("compilercompatibility/signalHandlers.qml"),
                                     &engine);
    if (QTest::currentTestFailed())
        return;

    // this is a "bare minimum" test, but if this succeeds, we could test other
    // things elsewhere
    QCOMPARE(runtimeFunctionIndicesFromCachegen, runtimeFunctionIndicesFromTypeCompiler);
    QCOMPARE(unitFromCachegen->runtimeFunctions.size(),
             unitFromTypeCompiler->runtimeFunctions.size());
    // make sure that units really come from different places (the machinery
    // could in theory be smart enough to figure the qmlcachegen cached
    // version), fairly questionable check but better than nothing
#ifdef Q_OS_ANDROID
    QSKIP("qrc and file system is the same thing on Android");
#endif
    QVERIFY(unitFromCachegen->url() != unitFromTypeCompiler->url());
}

void tst_QQMLTypeLoader::loadTypeOnShutdown()
{
    bool dead1 = false;
    bool dead2 = false;

    {
        QQmlEngine engine;
        auto good = new QQmlComponent(
                &engine, testFileUrl("doesExist.qml"),
                QQmlComponent::CompilationMode::Asynchronous, &engine);
        QObject::connect(
                good, &QQmlComponent::statusChanged, &engine,
                [&](QQmlComponent::Status) {

            // Must not call this if the engine is already dead.
            QVERIFY(engine.rootContext());

        });

        QObject::connect(good, &QQmlComponent::destroyed, good, [&]() { dead1 = true; });
        QVERIFY(good->isLoading());

        auto bad = new QQmlComponent(
                &engine, testFileUrl("doesNotExist.qml"),
                QQmlComponent::CompilationMode::Asynchronous, &engine);
        QObject::connect(
                bad, &QQmlComponent::statusChanged, &engine,
                [&](QQmlComponent::Status) {

            // Must not call this if the engine is already dead.
            // Must also not leak memory from the events the error produces.
            QVERIFY(engine.rootContext());

        });

        QObject::connect(bad, &QQmlComponent::destroyed, bad, [&]() { dead2 = true; });
        QVERIFY(bad->isLoading());
    }

    QVERIFY(dead1);
    QVERIFY(dead2);
}

void tst_QQMLTypeLoader::floodTypeLoaderEventQueue()
{
    QQmlEngine engine;

    // Flood the typeloader with useless messages.
    for (int i = 0; i < 1000; ++i) {
        QQmlComponent c(&engine);
        c.setData(QString::fromLatin1(R"(
                import "barf:/not/actually/there%1"
                SomeElement {}
            )").arg(i).toUtf8(), QUrl::fromLocalFile(QString::fromLatin1("foo%1.qml").arg(i)));
        QVERIFY(!c.isReady());
        // Should not crash when destrying the QQmlComponent.
    }
}

void tst_QQMLTypeLoader::retainQmlTypeAcrossEngines()
{
    QQmlEngine engine1;
    QQmlComponent component1(&engine1, testFileUrl("B.qml"));
    QVERIFY2(component1.isReady(), qPrintable(component1.errorString()));

    QQmlEngine engine2;
    QQmlComponent component2(&engine2, testFileUrl("B.qml"));
    QVERIFY2(component2.isReady(), qPrintable(component2.errorString()));

    QQmlEngine engine3;
    QQmlComponent component3(&engine3, testFileUrl("C.qml"));
    QVERIFY2(component3.isReady(), qPrintable(component3.errorString()));

    QQmlComponentPrivate *p1 = QQmlComponentPrivate::get(&component1);
    QVERIFY(p1);
    const auto cu1 = p1->compilationUnit;
    QVERIFY(cu1);

    QQmlComponentPrivate *p2 = QQmlComponentPrivate::get(&component2);
    QVERIFY(p2);
    const auto cu2 = p2->compilationUnit;
    QVERIFY(cu2);

    QQmlComponentPrivate *p3 = QQmlComponentPrivate::get(&component3);
    QVERIFY(p3);
    const auto cu3 = p3->compilationUnit;
    QVERIFY(cu3);

    // The _executable_ CUs are all different
    QVERIFY(cu1 != cu2);
    QVERIFY(cu1 != cu3);
    QVERIFY(cu2 != cu3);

    const auto base1 = cu1->baseCompilationUnit();
    const auto base2 = cu2->baseCompilationUnit();
    const auto base3 = cu3->baseCompilationUnit();

    QCOMPARE(base1, base2);
    QVERIFY(base1 != base3);
    QVERIFY(base2 != base3);

    const QQmlType qmltype1 = base1->qmlType;
    const QMetaObject *mo1 = qmltype1.typeId().metaObject();
    QVERIFY(mo1);

    const QQmlType qmltype3 = base3->qmlType;
    const QMetaObject *mo3 = qmltype3.typeId().metaObject();
    QVERIFY(mo3);

    QVERIFY(mo1 != mo3);

    // The base classes are all the same.
    QCOMPARE(mo1->superClass(), mo3->superClass());
}

QTEST_MAIN(tst_QQMLTypeLoader)

#include "tst_qqmltypeloader.moc"
