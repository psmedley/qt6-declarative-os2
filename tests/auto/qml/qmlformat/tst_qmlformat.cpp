// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QTemporaryDir>
#include <QtTest/private/qemulationdetector_p.h>
#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QtQmlDom/private/qqmldomitem_p.h>
#include <QtQmlDom/private/qqmldomlinewriter_p.h>
#include <QtQmlDom/private/qqmldomoutwriter_p.h>
#include <QtQmlDom/private/qqmldomtop_p.h>

using namespace QQmlJS::Dom;

// TODO refactor extension helpers
const QString QML_EXT = ".qml";
const QString JS_EXT = ".js";
const QString MJS_EXT = ".mjs";

static QStringView fileExt(QStringView filename)
{
    if (filename.endsWith(QML_EXT)) {
        return QML_EXT;
    }
    if (filename.endsWith(JS_EXT)) {
        return JS_EXT;
    }
    if (filename.endsWith(MJS_EXT)) {
        return MJS_EXT;
    }
    Q_UNREACHABLE();
};

class TestQmlformat: public QQmlDataTest
{
    Q_OBJECT

public:
    enum class RunOption { OnCopy, OrigToCopy };
    TestQmlformat();

private Q_SLOTS:
    void initTestCase() override;

    //actually testFormat tests CLI of qmlformat
    void testFormat();
    void testFormat_data();

    void testLineEndings();
#if !defined(QTEST_CROSS_COMPILED) // sources not available when cross compiled
    void testExample();
    void testExample_data();
    void normalizeExample();
    void normalizeExample_data();
#endif

    void testBackupFileLimit();

    void testFilesOption_data();
    void testFilesOption();

    void plainJS_data();
    void plainJS();

    void ecmascriptModule();

private:
    QString readTestFile(const QString &path);
    //TODO(QTBUG-117849) refactor this helper function
    QString runQmlformat(const QString &fileToFormat, QStringList args, bool shouldSucceed = true,
                         RunOption rOption = RunOption::OnCopy, QStringView ext = QML_EXT);
    QString formatInMemory(const QString &fileToFormat, bool *didSucceed = nullptr,
                           LineWriterOptions options = LineWriterOptions(),
                           WriteOutChecks extraChecks = WriteOutCheck::ReparseCompare,
                           WriteOutChecks largeChecks = WriteOutCheck::None);

    QString m_qmlformatPath;
    QStringList m_excludedDirs;
    QStringList m_invalidFiles;
    QStringList m_ignoreFiles;

    QStringList findFiles(const QDir &);
    bool isInvalidFile(const QFileInfo &fileName) const;
    bool isIgnoredFile(const QFileInfo &fileName) const;
};

// Don't fail on warnings because we read a lot of QML files that might intentionally be malformed.
TestQmlformat::TestQmlformat()
    : QQmlDataTest(QT_QMLTEST_DATADIR, FailOnWarningsPolicy::DoNotFailOnWarnings)
{
}

void TestQmlformat::initTestCase()
{
    QQmlDataTest::initTestCase();
    m_qmlformatPath = QLibraryInfo::path(QLibraryInfo::BinariesPath) + QLatin1String("/qmlformat");
#ifdef Q_OS_WIN
    m_qmlformatPath += QLatin1String(".exe");
#endif
    if (!QFileInfo(m_qmlformatPath).exists()) {
        QString message = QStringLiteral("qmlformat executable not found (looked for %0)").arg(m_qmlformatPath);
        QFAIL(qPrintable(message));
    }

    // Add directories you want excluded here

    // These snippets are not expected to run on their own.
    m_excludedDirs << "doc/src/snippets/qml/visualdatamodel_rootindex";
    m_excludedDirs << "doc/src/snippets/qml/qtbinding";
    m_excludedDirs << "doc/src/snippets/qml/imports";
    m_excludedDirs << "doc/src/snippets/qtquick1/visualdatamodel_rootindex";
    m_excludedDirs << "doc/src/snippets/qtquick1/qtbinding";
    m_excludedDirs << "doc/src/snippets/qtquick1/imports";
    m_excludedDirs << "tests/manual/v4";
    m_excludedDirs << "tests/manual/qmllsformatter";
    m_excludedDirs << "tests/auto/qml/ecmascripttests";
    m_excludedDirs << "tests/auto/qml/qmllint";

    // Add invalid files (i.e. files with syntax errors)
    m_invalidFiles << "tests/auto/quick/qquickloader/data/InvalidSourceComponent.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/signal.2.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/signal.3.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/signal.5.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/property.4.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/empty.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/missingObject.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/insertedSemicolon.1.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/nonexistantProperty.5.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/invalidRoot.1.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/invalidQmlEnumValue.1.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/invalidQmlEnumValue.2.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/invalidQmlEnumValue.3.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/invalidID.4.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/questionDotEOF.qml";
    m_invalidFiles << "tests/auto/qml/qquickfolderlistmodel/data/dummy.qml";
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/stringParsing_error.1.qml";
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/stringParsing_error.2.qml";
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/stringParsing_error.3.qml";
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/stringParsing_error.4.qml";
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/stringParsing_error.5.qml";
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/stringParsing_error.6.qml";
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/numberParsing_error.1.qml";
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/numberParsing_error.2.qml";
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/incrDecrSemicolon_error1.qml";
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/incrDecrSemicolon_error1.qml";
    m_invalidFiles << "tests/auto/qml/debugger/qqmlpreview/data/broken.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/fuzzed.2.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/fuzzed.3.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/requiredProperties.2.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/nullishCoalescing_LHS_And.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/nullishCoalescing_LHS_And.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/nullishCoalescing_LHS_Or.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/nullishCoalescing_RHS_And.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/nullishCoalescing_RHS_Or.qml";
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/typeAnnotations.2.qml";
    m_invalidFiles << "tests/auto/qml/qqmlparser/data/disallowedtypeannotations/qmlnestedfunction.qml";
    m_invalidFiles << "tests/auto/qmlls/utils/data/emptyFile.qml";
    m_invalidFiles << "tests/auto/qmlls/utils/data/completions/missingRHS.qml";
    m_invalidFiles << "tests/auto/qmlls/utils/data/completions/missingRHS.parserfail.qml";
    m_invalidFiles << "tests/auto/qmlls/utils/data/completions/attachedPropertyMissingRHS.qml";
    m_invalidFiles << "tests/auto/qmlls/utils/data/completions/groupedPropertyMissingRHS.qml";
    m_invalidFiles << "tests/auto/qmlls/utils/data/completions/afterDots.qml";
    m_invalidFiles << "tests/auto/qmlls/modules/data/completions/bindingAfterDot.qml";
    m_invalidFiles << "tests/auto/qmlls/modules/data/completions/defaultBindingAfterDot.qml";
    m_invalidFiles << "tests/auto/qmlls/utils/data/qualifiedModule.qml";

    // Files that get changed:
    // rewrite of import "bla/bla/.." to import "bla"
    m_invalidFiles << "tests/auto/qml/qqmlcomponent/data/componentUrlCanonicalization.4.qml";
    // block -> object in internal update
    m_invalidFiles << "tests/auto/qml/qqmlpromise/data/promise-executor-throw-exception.qml";
    // removal of unsupported indexing of Object declaration
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/hangOnWarning.qml";
    // removal of duplicated id
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/component.3.qml";
    // Optional chains are not permitted on the left-hand-side in assignments
    m_invalidFiles << "tests/auto/qml/qqmllanguage/data/optionalChaining.LHS.qml";
    // object literal with = assignements
    m_invalidFiles << "tests/auto/quickcontrols/controls/data/tst_scrollbar.qml";

    // These files rely on exact formatting
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/incrDecrSemicolon1.qml";
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/incrDecrSemicolon_error1.qml";
    m_invalidFiles << "tests/auto/qml/qqmlecmascript/data/incrDecrSemicolon2.qml";

    // These files are too big
    m_ignoreFiles << "tests/benchmarks/qml/qmldom/data/longQmlFile.qml";
    m_ignoreFiles << "tests/benchmarks/qml/qmldom/data/deeplyNested.qml";
}

QStringList TestQmlformat::findFiles(const QDir &d)
{
    for (int ii = 0; ii < m_excludedDirs.size(); ++ii) {
        QString s = m_excludedDirs.at(ii);
        if (d.absolutePath().endsWith(s))
            return QStringList();
    }

    QStringList rv;

    const QStringList files = d.entryList(QStringList() << QLatin1String("*.qml"),
                                          QDir::Files);
    for (const QString &file: files) {
        QString absoluteFilePath = d.absoluteFilePath(file);
        if (!isIgnoredFile(QFileInfo(absoluteFilePath)))
            rv << absoluteFilePath;
    }

    const QStringList dirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot |
                                         QDir::NoSymLinks);
    for (const QString &dir: dirs) {
        QDir sub = d;
        sub.cd(dir);
        rv << findFiles(sub);
    }

    return rv;
}

bool TestQmlformat::isInvalidFile(const QFileInfo &fileName) const
{
    for (const QString &invalidFile : m_invalidFiles) {
        if (fileName.absoluteFilePath().endsWith(invalidFile))
            return true;
    }
    return false;
}

bool TestQmlformat::isIgnoredFile(const QFileInfo &fileName) const
{
    for (const QString &file : m_ignoreFiles) {
        if (fileName.absoluteFilePath().endsWith(file))
            return true;
    }
    return false;
}

QString TestQmlformat::readTestFile(const QString &path)
{
    QFile file(testFile(path));

    if (!file.open(QIODevice::ReadOnly))
        return "";

    return QString::fromUtf8(file.readAll());
}

void TestQmlformat::testLineEndings()
{
    // macos
    const QString macosContents =
            runQmlformat(testFile("Example1.formatted.qml"), { "-l", "macos" });
    QVERIFY(!macosContents.contains("\n"));
    QVERIFY(macosContents.contains("\r"));

    // windows
    const QString windowsContents =
            runQmlformat(testFile("Example1.formatted.qml"), { "-l", "windows" });
    QVERIFY(windowsContents.contains("\r\n"));

    // unix
    const QString unixContents = runQmlformat(testFile("Example1.formatted.qml"), { "-l", "unix" });
    QVERIFY(unixContents.contains("\n"));
    QVERIFY(!unixContents.contains("\r"));
}

void TestQmlformat::testFormat_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("fileFormatted");
    QTest::addColumn<QStringList>("args");
    QTest::addColumn<RunOption>("runOption");

    QTest::newRow("example1") << "Example1.qml"
                              << "Example1.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("example1 (tabs)")
            << "Example1.qml"
            << "Example1.formatted.tabs.qml" << QStringList { "-t" } << RunOption::OnCopy;
    QTest::newRow("example1 (two spaces)")
            << "Example1.qml"
            << "Example1.formatted.2spaces.qml" << QStringList { "-w", "2" } << RunOption::OnCopy;
    QTest::newRow("annotation") << "Annotations.qml"
                                << "Annotations.formatted.qml" << QStringList {}
                                << RunOption::OnCopy;
    QTest::newRow("front inline") << "FrontInline.qml"
                                  << "FrontInline.formatted.qml" << QStringList {}
                                  << RunOption::OnCopy;
    QTest::newRow("if blocks") << "IfBlocks.qml"
                               << "IfBlocks.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("read-only properties")
            << "readOnlyProps.qml"
            << "readOnlyProps.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("states and transitions")
            << "statesAndTransitions.qml"
            << "statesAndTransitions.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("large bindings")
            << "largeBindings.qml"
            << "largeBindings.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("verbatim strings")
            << "verbatimString.qml"
            << "verbatimString.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("inline components")
            << "inlineComponents.qml"
            << "inlineComponents.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("nested ifs") << "nestedIf.qml"
                                << "nestedIf.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("QTBUG-85003") << "QtBug85003.qml"
                                 << "QtBug85003.formatted.qml" << QStringList {}
                                 << RunOption::OnCopy;
    QTest::newRow("nested functions")
            << "nestedFunctions.qml"
            << "nestedFunctions.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("multiline comments")
            << "multilineComment.qml"
            << "multilineComment.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("for of") << "forOf.qml"
                            << "forOf.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("property names")
            << "propertyNames.qml"
            << "propertyNames.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("empty object") << "emptyObject.qml"
                                  << "emptyObject.formatted.qml" << QStringList {}
                                  << RunOption::OnCopy;
    QTest::newRow("arrow functions")
            << "arrowFunctions.qml"
            << "arrowFunctions.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("settings") << "settings/Example1.qml"
                              << "settings/Example1.formatted_mac_cr.qml" << QStringList {}
                              << RunOption::OrigToCopy;
    QTest::newRow("forWithLet")
            << "forWithLet.qml"
            << "forWithLet.formatted.qml" << QStringList {} << RunOption::OnCopy;

    QTest::newRow("objects spacing (no changes)")
            << "objectsSpacing.qml"
            << "objectsSpacing.formatted.qml" << QStringList { "--objects-spacing" } << RunOption::OnCopy;

    QTest::newRow("normalize + objects spacing")
            << "normalizedObjectsSpacing.qml"
            << "normalizedObjectsSpacing.formatted.qml" << QStringList { "-n", "--objects-spacing" } << RunOption::OnCopy;

    QTest::newRow("ids new lines")
            << "checkIdsNewline.qml"
            << "checkIdsNewline.formatted.qml" << QStringList { "-n" } << RunOption::OnCopy;

    QTest::newRow("functions spacing (no changes)")
            << "functionsSpacing.qml"
            << "functionsSpacing.formatted.qml" << QStringList { "--functions-spacing" } << RunOption::OnCopy;

    QTest::newRow("normalize + functions spacing")
            << "normalizedFunctionsSpacing.qml"
            << "normalizedFunctionsSpacing.formatted.qml" << QStringList { "-n", "--functions-spacing" } << RunOption::OnCopy;
    QTest::newRow("dontRemoveComments")
            << "dontRemoveComments.qml"
            << "dontRemoveComments.formatted.qml" << QStringList {} << RunOption::OnCopy;
    QTest::newRow("ecmaScriptClassInQml")
            << "ecmaScriptClassInQml.qml"
            << "ecmaScriptClassInQml.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("arrowFunctionWithBinding")
            << "arrowFunctionWithBinding.qml"
            << "arrowFunctionWithBinding.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("blanklinesAfterComment")
            << "blanklinesAfterComment.qml"
            << "blanklinesAfterComment.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("pragmaValueList")
            << "pragma.qml"
            << "pragma.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("objectDestructuring")
            << "objectDestructuring.qml"
            << "objectDestructuring.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("destructuringFunctionParameter")
            << "destructuringFunctionParameter.qml"
            << "destructuringFunctionParameter.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("ellipsisFunctionArgument")
            << "ellipsisFunctionArgument.qml"
            << "ellipsisFunctionArgument.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("importStatements")
            << "importStatements.qml"
            << "importStatements.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("arrayEndComma")
            << "arrayEndComma.qml"
            << "arrayEndComma.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("escapeChars")
            << "escapeChars.qml"
            << "escapeChars.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("javascriptBlock")
            << "javascriptBlock.qml"
            << "javascriptBlock.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("enumWithValues")
            << "enumWithValues.qml"
            << "enumWithValues.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("typeAnnotatedSignal")
            << "signal.qml"
            << "signal.formatted.qml" << QStringList{} << RunOption::OnCopy;
    //plainJS
    QTest::newRow("nestedLambdaWithIfElse")
            << "lambdaWithIfElseInsideLambda.js"
            << "lambdaWithIfElseInsideLambda.formatted.js" << QStringList{} << RunOption::OnCopy;

    QTest::newRow("indentEquals2")
            << "threeFunctionsOneLine.js"
            << "threeFunctions.formattedW2.js" << QStringList{"-w=2"} << RunOption::OnCopy;

    QTest::newRow("tabIndents")
            << "threeFunctionsOneLine.js"
            << "threeFunctions.formattedTabs.js" << QStringList{"-t"} << RunOption::OnCopy;

    QTest::newRow("normalizedFunctionSpacing")
            << "threeFunctionsOneLine.js"
            << "threeFunctions.formattedFuncSpacing.js"
            << QStringList{ "-n", "--functions-spacing" } << RunOption::OnCopy;

    QTest::newRow("esm_tabIndents")
            << "mini_esm.mjs"
            << "mini_esm.formattedTabs.mjs" << QStringList{ "-t" } << RunOption::OnCopy;
    QTest::newRow("noSuperfluousSpaceInsertions")
            << "noSuperfluousSpaceInsertions.qml"
            << "noSuperfluousSpaceInsertions.formatted.qml" << QStringList{} << RunOption::OnCopy;
    QTest::newRow("noSuperfluousSpaceInsertions.fail_id")
            << "noSuperfluousSpaceInsertions.fail_id.qml"
            << "noSuperfluousSpaceInsertions.fail_id.formatted.qml"
            << QStringList{} << RunOption::OnCopy;
    QTest::newRow("noSuperfluousSpaceInsertions.fail_QtObject")
            << "noSuperfluousSpaceInsertions.fail_QtObject.qml"
            << "noSuperfluousSpaceInsertions.fail_QtObject.formatted.qml"
            << QStringList{} << RunOption::OnCopy;
    QTest::newRow("noSuperfluousSpaceInsertions.fail_signal")
            << "noSuperfluousSpaceInsertions.fail_signal.qml"
            << "noSuperfluousSpaceInsertions.fail_signal.formatted.qml"
            << QStringList{} << RunOption::OnCopy;
    QTest::newRow("noSuperfluousSpaceInsertions.fail_enum")
            << "noSuperfluousSpaceInsertions.fail_enum.qml"
            << "noSuperfluousSpaceInsertions.fail_enum.formatted.qml"
            << QStringList{} << RunOption::OnCopy;
    QTest::newRow("noSuperfluousSpaceInsertions.fail_parameters")
            << "noSuperfluousSpaceInsertions.fail_parameters.qml"
            << "noSuperfluousSpaceInsertions.fail_parameters.formatted.qml"
            << QStringList{} << RunOption::OnCopy;
    QTest::newRow("fromAsIdentifier")
            << "fromAsIdentifier.qml"
            << "fromAsIdentifier.formatted.qml"
            << QStringList{} << RunOption::OnCopy;
}

void TestQmlformat::testFormat()
{
    QFETCH(QString, file);
    QFETCH(QString, fileFormatted);
    QFETCH(QStringList, args);
    QFETCH(RunOption, runOption);

    auto formatted = runQmlformat(testFile(file), args, true, runOption, fileExt(file));
    QEXPECT_FAIL("normalizedFunctionSpacing",
                 "Normalize && function spacing are not yet supported for JS", Abort);
    QEXPECT_FAIL("noSuperfluousSpaceInsertions.fail_id",
                 "Not all cases have been covered yet (QTBUG-133315, QTBUG-123386)", Abort);
    QEXPECT_FAIL("noSuperfluousSpaceInsertions.fail_QtObject",
                 "Not all cases have been covered yet (QTBUG-133315, QTBUG-123386)", Abort);
    QEXPECT_FAIL("noSuperfluousSpaceInsertions.fail_signal",
                 "Not all cases have been covered yet (QTBUG-133315, QTBUG-123386)", Abort);
    QEXPECT_FAIL("noSuperfluousSpaceInsertions.fail_enum",
                 "Not all cases have been covered yet (QTBUG-133315, QTBUG-123386)", Abort);
    QEXPECT_FAIL("noSuperfluousSpaceInsertions.fail_parameters",
                 "Not all cases have been covered yet (QTBUG-133315, QTBUG-123386)", Abort);
    auto exp = readTestFile(fileFormatted);
    QCOMPARE(formatted, exp);
}

void TestQmlformat::plainJS_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("fileFormatted");

    QTest::newRow("simpleStatement") << "simpleJSStatement.js"
                                     << "simpleJSStatement.formatted.js";
    QTest::newRow("simpleFunction") << "simpleOnelinerJSFunc.js"
                                    << "simpleOnelinerJSFunc.formatted.js";
    QTest::newRow("simpleLoop") << "simpleLoop.js"
                                << "simpleLoop.formatted.js";
    QTest::newRow("messyIfStatement") << "messyIfStatement.js"
                                      << "messyIfStatement.formatted.js";
    QTest::newRow("lambdaFunctionWithLoop") << "lambdaFunctionWithLoop.js"
                                            << "lambdaFunctionWithLoop.formatted.js";
    QTest::newRow("lambdaWithIfElse") << "lambdaWithIfElse.js"
                                      << "lambdaWithIfElse.formatted.js";
    QTest::newRow("nestedLambdaWithIfElse") << "lambdaWithIfElseInsideLambda.js"
                                            << "lambdaWithIfElseInsideLambda.formatted.js";
    QTest::newRow("twoFunctions") << "twoFunctions.js"
                                  << "twoFunctions.formatted.js";
    QTest::newRow("pragma") << "pragma.js"
                            << "pragma.formatted.js";
    QTest::newRow("classConstructor") << "class.js"
                                      << "class.formatted.js";
    QTest::newRow("legacyDirectives") << "directives.js"
                                      << "directives.formatted.js";
    QTest::newRow("legacyDirectivesWithComments") << "directivesWithComments.js"
                                                  << "directivesWithComments.formatted.js";
    QTest::newRow("preserveOptionalTokens") << "preserveOptionalTokens.js"
                                            << "preserveOptionalTokens.formatted.js";
    QTest::newRow("noSuperfluousSpaceInsertions.fail_pragma")
            << "noSuperfluousSpaceInsertions.fail_pragma.js"
            << "noSuperfluousSpaceInsertions.fail_pragma.formatted.js";
    QTest::newRow("fromAsIdentifier") << "fromAsIdentifier.js"
                                      << "fromAsIdentifier.formatted.js";
}

void TestQmlformat::plainJS()
{
    QFETCH(QString, file);
    QFETCH(QString, fileFormatted);

    bool wasSuccessful;
    LineWriterOptions opts;
#ifdef Q_OS_WIN
    opts.lineEndings = QQmlJS::Dom::LineWriterOptions::LineEndings::Windows;
#endif
    QString output = formatInMemory(testFile(file), &wasSuccessful, opts, WriteOutCheck::None);

    QVERIFY(wasSuccessful && !output.isEmpty());

    // TODO(QTBUG-119404)
    QEXPECT_FAIL("classConstructor", "see QTBUG-119404", Abort);
    // TODO(QTBUG-119770)
    QEXPECT_FAIL("legacyDirectivesWithComments", "see QTBUG-119770", Abort);
    QEXPECT_FAIL("noSuperfluousSpaceInsertions.fail_pragma",
                 "Not all cases have been covered yet (QTBUG-133315)", Abort);
    auto exp = readTestFile(fileFormatted);
    QCOMPARE(output, exp);
}

void TestQmlformat::ecmascriptModule()
{
    QString file("esm.mjs");
    QString formattedFile("esm.formatted.mjs");

    bool wasSuccessful;
    LineWriterOptions opts;
#ifdef Q_OS_WIN
    opts.lineEndings = QQmlJS::Dom::LineWriterOptions::LineEndings::Windows;
#endif
    QString output = formatInMemory(testFile(file), &wasSuccessful, opts, WriteOutCheck::None);

    QVERIFY(wasSuccessful && !output.isEmpty());

    auto exp = readTestFile(formattedFile);
    QCOMPARE(output, readTestFile(formattedFile));
}

#if !defined(QTEST_CROSS_COMPILED) // sources not available when cross compiled
void TestQmlformat::testExample_data()
{
    if (QTestPrivate::isRunningArmOnX86())
        QSKIP("Crashes in QEMU. (timeout)");
    QTest::addColumn<QString>("file");

    QString examples = QLatin1String(SRCDIR) + "/../../../../examples/";
    QString tests = QLatin1String(SRCDIR) + "/../../../../tests/";

    QStringList exampleFiles;
    QStringList testFiles;
    QStringList files;
    exampleFiles << findFiles(QDir(examples));
    testFiles << findFiles(QDir(tests));

    // Actually this test is an e2e test and not the unit test.
    // At the moment of writing, CI lacks providing instruments for the automated tests
    // which might be time-consuming, as for example this one.
    // Therefore as part of QTBUG-122990 this test was copied to the /manual/e2e/qml/qmlformat
    // however very small fraction of the test data is still preserved here for the sake of
    // testing automatically at least a small part of the examples
    const int nBatch = 10;
    files << exampleFiles.mid(0, nBatch) << exampleFiles.mid(exampleFiles.size() / 2, nBatch)
          << exampleFiles.mid(exampleFiles.size() - nBatch, nBatch);
    files << testFiles.mid(0, nBatch) << testFiles.mid(exampleFiles.size() / 2, nBatch)
          << testFiles.mid(exampleFiles.size() - nBatch, nBatch);

    for (const QString &file : files)
        QTest::newRow(qPrintable(file)) << file;
}

void TestQmlformat::normalizeExample_data()
{
    if (QTestPrivate::isRunningArmOnX86())
        QSKIP("Crashes in QEMU. (timeout)");
    QTest::addColumn<QString>("file");

    QString examples = QLatin1String(SRCDIR) + "/../../../../examples/";
    QString tests = QLatin1String(SRCDIR) + "/../../../../tests/";

    // normalizeExample is similar to testExample, so we test it only on nExamples + nTests
    // files to avoid making too many
    QStringList files;
    const int nExamples = 10;
    int i = 0;
    for (const auto &f : findFiles(QDir(examples))) {
        files << f;
        if (++i == nExamples)
            break;
    }
    const int nTests = 10;
    i = 0;
    for (const auto &f : findFiles(QDir(tests))) {
        files << f;
        if (++i == nTests)
            break;
    }

    for (const QString &file : files)
        QTest::newRow(qPrintable(file)) << file;
}
#endif

#if !defined(QTEST_CROSS_COMPILED) // sources not available when cross compiled
void TestQmlformat::testExample()
{
    QFETCH(QString, file);
    const bool isInvalid = isInvalidFile(QFileInfo(file));
    bool wasSuccessful;
    LineWriterOptions opts;
    opts.attributesSequence = LineWriterOptions::AttributesSequence::Preserve;
    QString output = formatInMemory(file, &wasSuccessful, opts);

    if (!isInvalid)
        QVERIFY(wasSuccessful && !output.isEmpty());
}

void TestQmlformat::normalizeExample()
{
    QFETCH(QString, file);
    const bool isInvalid = isInvalidFile(QFileInfo(file));
    bool wasSuccessful;
    LineWriterOptions opts;
    opts.attributesSequence = LineWriterOptions::AttributesSequence::Normalize;
    QString output = formatInMemory(file, &wasSuccessful, opts);

    if (!isInvalid)
        QVERIFY(wasSuccessful && !output.isEmpty());
}
#endif

void TestQmlformat::testBackupFileLimit()
{
    // Create a temporary directory
    QTemporaryDir tempDir;

    // Unformatted file to format
    const QString fileToFormat{ testFile("Annotations.qml") };

    {
        const QString tempFile = tempDir.path() + QDir::separator() + "test_0.qml";
        const QString backupFile = tempFile + QStringLiteral("~");
        QFile::copy(fileToFormat, tempFile);

        QProcess process;
        process.start(m_qmlformatPath, QStringList{ "--verbose", "--inplace", tempFile });
        QVERIFY(process.waitForFinished());
        QCOMPARE(process.exitStatus(), QProcess::NormalExit);
        QCOMPARE(process.exitCode(), 0);
        QVERIFY(QFileInfo::exists(tempFile));
        QVERIFY(!QFileInfo::exists(backupFile));
    };
}

void TestQmlformat::testFilesOption_data()
{
    QTest::addColumn<QString>("containerFile");
    QTest::addColumn<QStringList>("individualFiles");

    QTest::newRow("initial") << "fileListToFormat" << QStringList{ "valid1.qml", "valid2.qml" };
}

void TestQmlformat::testFilesOption()
{
    QFETCH(QString, containerFile);
    QFETCH(QStringList, individualFiles);

    // Create a temporary directory
    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);
    QStringList actualFormattedFilesPath;

    // Iterate through files in the source directory and copy them to the temporary directory
    const auto sourceDir = dataDirectory() + QDir::separator() + "filesOption";

    // Create a file that contains the list of files to be formatted
    const QString tempFilePath = tempDir.path() + QDir::separator() + containerFile;
    QFile container(tempFilePath);
    if (container.open(QIODevice::Text | QIODevice::WriteOnly)) {
        QTextStream out(&container);

        for (const auto &file : individualFiles) {
            QString destinationFilePath = tempDir.path() + QDir::separator() + file;
            if (QFile::copy(sourceDir + QDir::separator() + file, destinationFilePath))
                actualFormattedFilesPath << destinationFilePath;
            out << destinationFilePath << "\n";
        }

        container.close();
    } else {
        QFAIL("Cannot create temp test file\n");
        return;
    }

    {
        QProcess process;
        process.start(m_qmlformatPath, QStringList{"-F", tempFilePath});
        QVERIFY(process.waitForFinished());
        QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    }

    const auto readFile = [](const QString &filePath){
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Error on opening the file " << filePath;
            return QByteArray{};
        }

        return file.readAll();
    };

    for (const auto &filePath : actualFormattedFilesPath) {
        auto expectedFormattedFile = QFileInfo(filePath).fileName();
        const auto expectedFormattedFilePath = sourceDir + QDir::separator() +
            expectedFormattedFile.replace(".qml", ".formatted.qml");

        QCOMPARE(readFile(filePath), readFile(expectedFormattedFilePath));
    }
}

QString TestQmlformat::runQmlformat(const QString &fileToFormat, QStringList args,
                                    bool shouldSucceed, RunOption rOptions, QStringView ext)
{
    // Copy test file to temporary location
    QTemporaryDir tempDir;
    const QString tempFile = (tempDir.path() + QDir::separator() + "to_format") % ext;

    if (rOptions == RunOption::OnCopy) {
        QFile::copy(fileToFormat, tempFile);
        args << QLatin1String("-i");
        args << tempFile;
    } else {
        args << fileToFormat;
    }

    auto verify = [&]() {
        QProcess process;
        if (rOptions == RunOption::OrigToCopy)
            process.setStandardOutputFile(tempFile);
        process.start(m_qmlformatPath, args);
        QVERIFY(process.waitForFinished());
        QCOMPARE(process.exitStatus(), QProcess::NormalExit);
        if (shouldSucceed)
            QCOMPARE(process.exitCode(), 0);
    };
    verify();

    QFile temp(tempFile);

    if (!temp.open(QIODevice::ReadOnly))
        qFatal("Could not open %s", qPrintable(tempFile));
    QString formatted = QString::fromUtf8(temp.readAll());

    return formatted;
}

QString TestQmlformat::formatInMemory(const QString &fileToFormat, bool *didSucceed,
                                      LineWriterOptions options, WriteOutChecks extraChecks,
                                      WriteOutChecks largeChecks)
{
    auto env = DomEnvironment::create(
            QStringList(), // as we load no dependencies we do not need any paths
            QQmlJS::Dom::DomEnvironment::Option::SingleThreaded
                    | QQmlJS::Dom::DomEnvironment::Option::NoDependencies);
    DomItem tFile;
    env->loadFile(FileToLoad::fromFileSystem(env, fileToFormat),
                  [&tFile](Path, const DomItem &, const DomItem &newIt) { tFile = newIt; });
    env->loadPendingDependencies();
    MutableDomItem myFile = tFile.field(Fields::currentItem);

    bool writtenOut;
    QString resultStr;
    if (myFile.field(Fields::isValid).value().toBool()) {
        WriteOutChecks checks = extraChecks;
        const qsizetype largeFileSize = 32000;
        if (tFile.field(Fields::code).value().toString().size() > largeFileSize)
            checks = largeChecks;

        QTextStream res(&resultStr);
        LineWriter lw([&res](QStringView s) { res << s; }, QLatin1String("*testStream*"), options);
        OutWriter ow(lw);
        ow.indentNextlines = true;
        DomItem qmlFile = tFile.field(Fields::currentItem);
        writtenOut = qmlFile.writeOutForFile(ow, checks);
        lw.eof();
        res.flush();
    }
    if (didSucceed)
        *didSucceed = writtenOut;
    return resultStr;
}

QTEST_MAIN(TestQmlformat)
#include "tst_qmlformat.moc"
