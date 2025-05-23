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
#include <QtQuickTest/quicktest.h>

#include <QtQuick/qquickview.h>
#include <QtQuick/private/qquicktableview_p.h>
#include <QtQuick/private/qquicktableview_p_p.h>
#include <QtQuick/private/qquickloader_p.h>

#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlexpression.h>
#include <QtQml/qqmlincubator.h>
#include <QtQmlModels/private/qqmlobjectmodel_p.h>
#include <QtQmlModels/private/qqmllistmodel_p.h>

#include "testmodel.h"

#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QtQuickTestUtils/private/viewtestutils_p.h>
#include <QtQuickTestUtils/private/visualtestutils_p.h>

using namespace QQuickViewTestUtils;
using namespace QQuickVisualTestUtils;

static const char* kDelegateObjectName = "tableViewDelegate";
static const char *kDelegatesCreatedCountProp = "delegatesCreatedCount";
static const char *kModelDataBindingProp = "modelDataBinding";

Q_DECLARE_METATYPE(QMarginsF);

#define GET_QML_TABLEVIEW(PROPNAME) \
    auto PROPNAME = view->rootObject()->property(#PROPNAME).value<QQuickTableView *>(); \
    QVERIFY(PROPNAME); \
    auto PROPNAME ## Private = QQuickTableViewPrivate::get(PROPNAME); \
    Q_UNUSED(PROPNAME ## Private)

#define LOAD_TABLEVIEW(fileName) \
    view->setSource(testFileUrl(fileName)); \
    view->show(); \
    QVERIFY(QTest::qWaitForWindowActive(view)); \
    GET_QML_TABLEVIEW(tableView)

#define LOAD_TABLEVIEW_ASYNC(fileName) \
    view->setSource(testFileUrl("asyncloader.qml")); \
    view->show(); \
    QVERIFY(QTest::qWaitForWindowActive(view)); \
    auto loader = view->rootObject()->property("loader").value<QQuickLoader *>(); \
    loader->setSource(testFileUrl(fileName)); \
    QTRY_VERIFY(loader->item()); \
    QCOMPARE(loader->status(), QQuickLoader::Status::Ready); \
    GET_QML_TABLEVIEW(tableView)

#define WAIT_UNTIL_POLISHED_ARG(item) \
    QVERIFY(QQuickTest::qIsPolishScheduled(item)); \
    QVERIFY(QQuickTest::qWaitForItemPolished(item))
#define WAIT_UNTIL_POLISHED WAIT_UNTIL_POLISHED_ARG(tableView)

class tst_QQuickTableView : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_QQuickTableView();

    QQuickTableViewAttached *getAttachedObject(const QObject *object) const;
    QPoint getContextRowAndColumn(const QQuickItem *item) const;

private:
    QQuickView *view = nullptr;

private slots:
    void initTestCase() override;
    void cleanupTestCase();

    void setAndGetModel_data();
    void setAndGetModel();
    void emptyModel_data();
    void emptyModel();
    void checkPreload_data();
    void checkPreload();
    void checkZeroSizedDelegate();
    void checkImplicitSizeDelegate();
    void checkColumnWidthWithoutProvider();
    void checkColumnWidthAndRowHeightFunctions();
    void checkDelegateWithAnchors();
    void checkColumnWidthProvider();
    void checkColumnWidthProviderInvalidReturnValues();
    void checkColumnWidthProviderNegativeReturnValue();
    void checkColumnWidthProviderNotCallable();
    void checkRowHeightWithoutProvider();
    void checkRowHeightProvider();
    void checkRowHeightProviderInvalidReturnValues();
    void checkRowHeightProviderNegativeReturnValue();
    void checkRowHeightProviderNotCallable();
    void isColumnLoadedAndIsRowLoaded();
    void checkForceLayoutFunction();
    void checkForceLayoutEndUpDoingALayout();
    void checkForceLayoutInbetweenAddingRowsToModel();
    void checkForceLayoutWhenAllItemsAreHidden();
    void checkContentWidthAndHeight();
    void checkContentWidthAndHeightForSmallTables();
    void checkPageFlicking();
    void checkExplicitContentWidthAndHeight();
    void checkExtents_origin();
    void checkExtents_endExtent();
    void checkExtents_moveTableToEdge();
    void checkContentXY();
    void noDelegate();
    void changeDelegateDuringUpdate();
    void changeModelDuringUpdate();
    void countDelegateItems_data();
    void countDelegateItems();
    void checkLayoutOfEqualSizedDelegateItems_data();
    void checkLayoutOfEqualSizedDelegateItems();
    void checkFocusRemoved_data();
    void checkFocusRemoved();
    void fillTableViewButNothingMore_data();
    void fillTableViewButNothingMore();
    void checkInitialAttachedProperties_data();
    void checkInitialAttachedProperties();
    void checkSpacingValues();
    void checkDelegateParent();
    void flick_data();
    void flick();
    void flickOvershoot_data();
    void flickOvershoot();
    void checkRowColumnCount();
    void modelSignals();
    void checkModelSignalsUpdateLayout();
    void dataChangedSignal();
    void checkThatPoolIsDrainedWhenReuseIsFalse();
    void checkIfDelegatesAreReused_data();
    void checkIfDelegatesAreReused();
    void checkIfDelegatesAreReusedAsymmetricTableSize();
    void checkContextProperties_data();
    void checkContextProperties();
    void checkContextPropertiesQQmlListProperyModel_data();
    void checkContextPropertiesQQmlListProperyModel();
    void checkRowAndColumnChangedButNotIndex();
    void checkThatWeAlwaysEmitChangedUponItemReused();
    void checkChangingModelFromDelegate();
    void checkRebuildViewportOnly();
    void useDelegateChooserWithoutDefault();
    void checkTableviewInsideAsyncLoader();
    void hideRowsAndColumns_data();
    void hideRowsAndColumns();
    void hideAndShowFirstColumn();
    void hideAndShowFirstRow();
    void checkThatRevisionedPropertiesCannotBeUsedInOldImports();
    void checkSyncView_rootView_data();
    void checkSyncView_rootView();
    void checkSyncView_childViews_data();
    void checkSyncView_childViews();
    void checkSyncView_differentSizedModels();
    void checkSyncView_differentGeometry();
    void checkSyncView_connect_late_data();
    void checkSyncView_connect_late();
    void checkSyncView_pageFlicking();
    void checkSyncView_emptyModel();
    void checkSyncView_topLeftChanged();
    void delegateWithRequiredProperties();
    void checkThatFetchMoreIsCalledWhenScrolledToTheEndOfTable();
    void replaceModel();
    void cellAtPos_data();
    void cellAtPos();
    void positionViewAtRow_data();
    void positionViewAtRow();
    void positionViewAtColumn_data();
    void positionViewAtColumn();
    void positionViewAtRowClamped_data();
    void positionViewAtRowClamped();
    void positionViewAtColumnClamped_data();
    void positionViewAtColumnClamped();
    void positionViewAtLastRow_data();
    void positionViewAtLastRow();
    void positionViewAtLastColumn_data();
    void positionViewAtLastColumn();
    void itemAtCell_data();
    void itemAtCell();
    void leftRightTopBottomProperties_data();
    void leftRightTopBottomProperties();
    void checkContentSize_data();
    void checkContentSize();
    void checkSelectionModelWithRequiredSelectedProperty_data();
    void checkSelectionModelWithRequiredSelectedProperty();
    void checkSelectionModelWithUnrequiredSelectedProperty();
    void removeAndAddSelectionModel();
    void testSelectableStartPosEndPos_data();
    void testSelectableStartPosEndPos();
    void testSelectableStartPosEndPosOutsideView();
    void testSelectableScrollTowardsPos();
    void resettingRolesRespected();
    void deletedDelegate();
    void checkRebuildJsModel();
};

tst_QQuickTableView::tst_QQuickTableView()
    : QQmlDataTest(QT_QMLTEST_DATADIR)
{
}

void tst_QQuickTableView::initTestCase()
{
    QQmlDataTest::initTestCase();
    qmlRegisterType<TestModel>("TestModel", 0, 1, "TestModel");
    view = createView();
}

void tst_QQuickTableView::cleanupTestCase()
{
    delete view;
}

QQuickTableViewAttached *tst_QQuickTableView::getAttachedObject(const QObject *object) const
{
    QObject *attachedObject = qmlAttachedPropertiesObject<QQuickTableView>(object);
    return static_cast<QQuickTableViewAttached *>(attachedObject);
}

QPoint tst_QQuickTableView::getContextRowAndColumn(const QQuickItem *item) const
{
    const auto context = qmlContext(item);
    const int row = context->contextProperty("row").toInt();
    const int column = context->contextProperty("column").toInt();
    return QPoint(column, row);
}

void tst_QQuickTableView::setAndGetModel_data()
{
    QTest::addColumn<QVariant>("model");

    QTest::newRow("QAIM 1x1") << TestModelAsVariant(1, 1);
    QTest::newRow("Number model 1") << QVariant::fromValue(1);
    QTest::newRow("QStringList 1") << QVariant::fromValue(QStringList() << "one");
}

void tst_QQuickTableView::setAndGetModel()
{
    // Test that we can set and get different kind of models
    QFETCH(QVariant, model);
    LOAD_TABLEVIEW("plaintableview.qml");

    tableView->setModel(model);
    QCOMPARE(model, tableView->model());
}

void tst_QQuickTableView::emptyModel_data()
{
    QTest::addColumn<QVariant>("model");

    QTest::newRow("QAIM") << TestModelAsVariant(0, 0);
    QTest::newRow("Number model") << QVariant::fromValue(0);
    QTest::newRow("QStringList") << QVariant::fromValue(QStringList());
}

void tst_QQuickTableView::emptyModel()
{
    // Check that if we assign an empty model to
    // TableView, no delegate items will be created.
    QFETCH(QVariant, model);
    LOAD_TABLEVIEW("plaintableview.qml");

    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableViewPrivate->loadedItems.count(), 0);
}

void tst_QQuickTableView::checkPreload_data()
{
    QTest::addColumn<bool>("reuseItems");

    QTest::newRow("reuse") << true;
    QTest::newRow("not reuse") << false;
}

void tst_QQuickTableView::checkPreload()
{
    // Check that the reuse pool is filled up with one extra row and
    // column (pluss corner) after rebuilding the table, but only if we reuse items.
    QFETCH(bool, reuseItems);
    LOAD_TABLEVIEW("plaintableview.qml");

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);
    tableView->setReuseItems(reuseItems);

    WAIT_UNTIL_POLISHED;

    if (reuseItems) {
        const int rowCount = tableViewPrivate->loadedRows.count();
        const int columnCount = tableViewPrivate->loadedColumns.count();
        const int expectedPoolSize = rowCount + columnCount + 1;
        QCOMPARE(tableViewPrivate->tableModel->poolSize(), expectedPoolSize);
    } else {
        QCOMPARE(tableViewPrivate->tableModel->poolSize(), 0);
    }
}

void tst_QQuickTableView::checkZeroSizedDelegate()
{
    // Check that if we assign a delegate with empty width and height, we
    // fall back to use kDefaultColumnWidth and kDefaultRowHeight as
    // column/row sizes.
    LOAD_TABLEVIEW("plaintableview.qml");

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    view->rootObject()->setProperty("delegateWidth", 0);
    view->rootObject()->setProperty("delegateHeight", 0);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*implicit"));

    WAIT_UNTIL_POLISHED;

    auto items = tableViewPrivate->loadedItems;
    QVERIFY(!items.isEmpty());

    for (auto fxItem : tableViewPrivate->loadedItems) {
        auto item = fxItem->item;
        QCOMPARE(item->width(), kDefaultColumnWidth);
        QCOMPARE(item->height(), kDefaultRowHeight);
    }
}

void tst_QQuickTableView::checkImplicitSizeDelegate()
{
    // Check that we can set the size of delegate items using
    // implicit width/height, instead of forcing the user to
    // create an attached object by using implicitWidth/Height.
    LOAD_TABLEVIEW("tableviewimplicitsize.qml");

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    auto items = tableViewPrivate->loadedItems;
    QVERIFY(!items.isEmpty());

    for (auto fxItem : tableViewPrivate->loadedItems) {
        auto item = fxItem->item;
        QCOMPARE(item->width(), 90);
        QCOMPARE(item->height(), 60);
    }
}

void tst_QQuickTableView::checkColumnWidthWithoutProvider()
{
    // Checks that a function isn't assigned to the columnWidthProvider property
    // and that the column width is then equal to sizeHintForColumn.
    LOAD_TABLEVIEW("alternatingrowheightcolumnwidth.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);
    QVERIFY(tableView->columnWidthProvider().isUndefined());

    WAIT_UNTIL_POLISHED;

    for (const int column : tableViewPrivate->loadedColumns.keys()) {
        const qreal expectedColumnWidth = tableViewPrivate->sizeHintForColumn(column);
        for (const int row : tableViewPrivate->loadedRows.keys()) {
            const auto item = tableViewPrivate->loadedTableItem(QPoint(column, row))->item;
            QCOMPARE(item->width(), expectedColumnWidth);
        }
    }
}

void tst_QQuickTableView::checkColumnWidthAndRowHeightFunctions()
{
    // Checks that the column width and row height functions return
    // the correct sizes. When we have row-, or columnWidthProviders
    // the actual row and column sizes will normally differ from the
    // minimum row and column sizes (which is the maximum implicit
    // size found among the delegates).
    LOAD_TABLEVIEW("userowcolumnprovider.qml");

    const int count = 4;
    auto model = TestModelAsVariant(count, count);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    const qreal expectedimplicitSize = 20;

    for (int i = 0; i < count; ++i) {
        const qreal expectedSize = i + 10;
        QCOMPARE(tableView->columnWidth(i), expectedSize);
        QCOMPARE(tableView->rowHeight(i), expectedSize);
        QCOMPARE(tableView->implicitColumnWidth(i), expectedimplicitSize);
        QCOMPARE(tableView->implicitRowHeight(i), expectedimplicitSize);
    }
}

void tst_QQuickTableView::checkDelegateWithAnchors()
{
    // Checks that we issue a warning if the delegate has anchors
    LOAD_TABLEVIEW("delegatewithanchors.qml");

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*anchors"));

    auto model = TestModelAsVariant(1, 1);
    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;
}

void tst_QQuickTableView::checkColumnWidthProvider()
{
    // Check that you can assign a function to the columnWidthProvider property, and
    // that it's used to control (and override) the width of the columns.
    LOAD_TABLEVIEW("userowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);
    QVERIFY(tableView->columnWidthProvider().isCallable());

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems) {
        // expectedWidth mirrors the expected return value of the assigned javascript function
        qreal expectedWidth = fxItem->cell.x() + 10;
        QCOMPARE(fxItem->item->width(), expectedWidth);
    }
}

void tst_QQuickTableView::checkColumnWidthProviderInvalidReturnValues()
{
    // Check that we fall back to use default columns widths, if you
    // assign a function to columnWidthProvider that returns invalid values.
    LOAD_TABLEVIEW("usefaultyrowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*implicit.*zero"));

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->width(), kDefaultColumnWidth);
}

void tst_QQuickTableView::checkColumnWidthProviderNegativeReturnValue()
{
    // Check that we fall back to use the implicit width of the delegate
    // items if the columnWidthProvider return a negative number.
    LOAD_TABLEVIEW("userowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);
    view->rootObject()->setProperty("returnNegativeColumnWidth", true);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->width(), 20);
}

void tst_QQuickTableView::checkColumnWidthProviderNotCallable()
{
    // Check that we fall back to use default columns widths, if you
    // assign something to columnWidthProvider that is not callable.
    LOAD_TABLEVIEW("usefaultyrowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);
    tableView->setRowHeightProvider(QJSValue());
    tableView->setColumnWidthProvider(QJSValue(10));

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".Provider.*function"));

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->width(), kDefaultColumnWidth);
}

void tst_QQuickTableView::checkRowHeightWithoutProvider()
{
    // Checks that a function isn't assigned to the rowHeightProvider property
    // and that the row height is then equal to sizeHintForRow.
    LOAD_TABLEVIEW("alternatingrowheightcolumnwidth.qml");

    auto model = TestModelAsVariant(10, 10);
    QVERIFY(tableView->rowHeightProvider().isUndefined());

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    for (const int row : tableViewPrivate->loadedRows.keys()) {
        const qreal expectedRowHeight = tableViewPrivate->sizeHintForRow(row);
        for (const int column : tableViewPrivate->loadedColumns.keys()) {
            const auto item = tableViewPrivate->loadedTableItem(QPoint(column, row))->item;
            QCOMPARE(item->height(), expectedRowHeight);
        }
    }
}

void tst_QQuickTableView::checkRowHeightProvider()
{
    // Check that you can assign a function to the columnWidthProvider property, and
    // that it's used to control (and override) the width of the columns.
    LOAD_TABLEVIEW("userowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);
    QVERIFY(tableView->rowHeightProvider().isCallable());

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems) {
        // expectedWidth mirrors the expected return value of the assigned javascript function
        qreal expectedHeight = fxItem->cell.y() + 10;
        QCOMPARE(fxItem->item->height(), expectedHeight);
    }
}

void tst_QQuickTableView::checkRowHeightProviderInvalidReturnValues()
{
    // Check that we fall back to use default row heights, if you
    // assign a function to rowHeightProvider that returns invalid values.
    LOAD_TABLEVIEW("usefaultyrowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*implicit.*zero"));

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->height(), kDefaultRowHeight);
}

void tst_QQuickTableView::checkRowHeightProviderNegativeReturnValue()
{
    // Check that we fall back to use the implicit height of the delegate
    // items if the rowHeightProvider return a negative number.
    LOAD_TABLEVIEW("userowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);
    view->rootObject()->setProperty("returnNegativeRowHeight", true);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->height(), 20);
}

void tst_QQuickTableView::checkRowHeightProviderNotCallable()
{
    // Check that we fall back to use default row heights, if you
    // assign something to rowHeightProvider that is not callable.
    LOAD_TABLEVIEW("usefaultyrowcolumnprovider.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);

    tableView->setColumnWidthProvider(QJSValue());
    tableView->setRowHeightProvider(QJSValue(10));

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*Provider.*function"));

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->height(), kDefaultRowHeight);
}

void tst_QQuickTableView::isColumnLoadedAndIsRowLoaded()
{
    // Check that all the delegate items are loaded and available from
    // the columnWidthProvider/rowHeightProvider when 'isColumnLoaded()'
    // and 'isRowLoaded()' returns true.
    LOAD_TABLEVIEW("iscolumnloaded.qml");

    auto model = TestModelAsVariant(4, 5);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    const int itemsInColumnAfterLoaded = view->rootObject()->property("itemsInColumnAfterLoaded").toInt();
    const int itemsInRowAfterLoaded = view->rootObject()->property("itemsInRowAfterLoaded").toInt();

    QCOMPARE(itemsInColumnAfterLoaded, tableView->rows());
    QCOMPARE(itemsInRowAfterLoaded, tableView->columns());
}

void tst_QQuickTableView::checkForceLayoutFunction()
{
    // When we set the 'columnWidths' property in the test file, the
    // columnWidthProvider should return other values than it did during
    // start-up. Check that this takes effect after a call to the 'forceLayout()' function.
    LOAD_TABLEVIEW("forcelayout.qml");

    const char *propertyName = "columnWidths";
    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // Check that the initial column widths are as specified in the QML file
    const qreal initialColumnWidth = view->rootObject()->property(propertyName).toReal();
    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->width(), initialColumnWidth);

    // Change the return value from the columnWidthProvider to something else
    const qreal newColumnWidth = 100;
    view->rootObject()->setProperty(propertyName, newColumnWidth);
    tableView->forceLayout();
    // We don't have to polish; The re-layout happens immediately

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->width(), newColumnWidth);
}

void tst_QQuickTableView::checkForceLayoutEndUpDoingALayout()
{
    // QTBUG-77074
    // Check that we change the implicit size of the delegate after
    // the initial loading, and at the same time hide some rows or
    // columns, and then do a forceLayout(), we end up with a
    // complete relayout that respects the new implicit size.
    LOAD_TABLEVIEW("tweakimplicitsize.qml");

    auto model = TestModelAsVariant(10, 10);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    const qreal newDelegateSize = 20;
    view->rootObject()->setProperty("delegateSize", newDelegateSize);
    // Hide a row, just to force the following relayout to
    // do a complete reload (and not just a relayout)
    view->rootObject()->setProperty("hideRow", 1);
    tableView->forceLayout();

    for (auto fxItem : tableViewPrivate->loadedItems)
        QCOMPARE(fxItem->item->height(), newDelegateSize);

    // Check that the content height has been updated as well
    const qreal rowSpacing = tableView->rowSpacing();
    const qreal colSpacing = tableView->columnSpacing();
    QCOMPARE(tableView->contentWidth(), (10 * (newDelegateSize + colSpacing)) - colSpacing);
    QCOMPARE(tableView->contentHeight(), (9 * (newDelegateSize + rowSpacing)) - rowSpacing);
}

void tst_QQuickTableView::checkForceLayoutInbetweenAddingRowsToModel()
{
    // Check that TableView doesn't assert if we call forceLayout() while waiting
    // for a callback from the model that the row count has changed. Also make sure
    // that we don't move the contentItem while doing so.
    LOAD_TABLEVIEW("plaintableview.qml");

    const int initialRowCount = 10;
    TestModel model(initialRowCount, 10);
    tableView->setModel(QVariant::fromValue(&model));

    connect(&model, &QAbstractItemModel::rowsInserted, [=](){
        QCOMPARE(tableView->rows(), initialRowCount);
        tableView->forceLayout();
        QCOMPARE(tableView->rows(), initialRowCount + 1);
    });

    WAIT_UNTIL_POLISHED;

    const int contentY = 10;
    tableView->setContentY(contentY);
    QCOMPARE(tableView->rows(), initialRowCount);
    QCOMPARE(tableView->contentY(), contentY);
    model.addRow(0);
    QCOMPARE(tableView->rows(), initialRowCount + 1);
    QCOMPARE(tableView->contentY(), contentY);
}

void tst_QQuickTableView::checkForceLayoutWhenAllItemsAreHidden()
{
    // Check that you can have a TableView where all columns are
    // initially hidden, and then show some columns and call
    // forceLayout(). This should make the columns become visible.
    LOAD_TABLEVIEW("forcelayout.qml");

    // Tell all columns to be hidden
    const char *propertyName = "columnWidths";
    view->rootObject()->setProperty(propertyName, 0);

    const int rows = 3;
    const int columns = 3;
    auto model = TestModelAsVariant(rows, columns);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // Check that the we have no items loaded
    QCOMPARE(tableViewPrivate->loadedColumns.count(), 0);
    QCOMPARE(tableViewPrivate->loadedRows.count(), 0);
    QCOMPARE(tableViewPrivate->loadedItems.count(), 0);

    // Tell all columns to be visible
    view->rootObject()->setProperty(propertyName, 10);
    tableView->forceLayout();

    QCOMPARE(tableViewPrivate->loadedRows.count(), rows);
    QCOMPARE(tableViewPrivate->loadedColumns.count(), columns);
    QCOMPARE(tableViewPrivate->loadedItems.count(), rows * columns);
}

void tst_QQuickTableView::checkContentWidthAndHeight()
{
    // Check that contentWidth/Height reports the correct size of the
    // table, based on knowledge of the rows and columns that has been loaded.
    LOAD_TABLEVIEW("contentwidthheight.qml");

    // Vertical and horizontal properties should be mirrored, so we only have
    // to do the calculations once, and use them for both axis, below.
    QCOMPARE(tableView->width(), tableView->height());
    QCOMPARE(tableView->rowSpacing(), tableView->columnSpacing());

    const int tableSize = 100;
    const int cellSizeSmall = 100;
    const int spacing = 1;
    auto model = TestModelAsVariant(tableSize, tableSize);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    const qreal expectedSizeInit = (tableSize * cellSizeSmall) + ((tableSize - 1) * spacing);
    QCOMPARE(tableView->contentWidth(), expectedSizeInit);
    QCOMPARE(tableView->contentHeight(), expectedSizeInit);
    QCOMPARE(tableViewPrivate->averageEdgeSize.width(), cellSizeSmall);
    QCOMPARE(tableViewPrivate->averageEdgeSize.height(), cellSizeSmall);

    // Flick to the end, and check that content width/height stays unchanged
    tableView->setContentX(tableView->contentWidth() - tableView->width());
    tableView->setContentY(tableView->contentHeight() - tableView->height());

    QCOMPARE(tableView->contentWidth(), expectedSizeInit);
    QCOMPARE(tableView->contentHeight(), expectedSizeInit);

    // Flick back to start
    tableView->setContentX(0);
    tableView->setContentY(0);

    // Since we move the viewport more than a page, tableview
    // will jump to the new position and do a rebuild.
    QVERIFY(tableViewPrivate->polishScheduled);
    QVERIFY(tableViewPrivate->scheduledRebuildOptions);
    WAIT_UNTIL_POLISHED;

    // We should still have the same content width/height as when we started
    QCOMPARE(tableView->contentWidth(), expectedSizeInit);
    QCOMPARE(tableView->contentHeight(), expectedSizeInit);
}

void tst_QQuickTableView::checkContentWidthAndHeightForSmallTables()
{
    // For tables where all the columns in the model are loaded, we know
    // the exact table width, and can therefore update the content width
    // if e.g new rows are added or removed. The same is true for rows.
    // This test will check that we do so.
    LOAD_TABLEVIEW("sizefromdelegate.qml");

    TestModel model(3, 3);
    tableView->setModel(QVariant::fromValue(&model));
    WAIT_UNTIL_POLISHED;

    const qreal initialContentWidth = tableView->contentWidth();
    const qreal initialContentHeight = tableView->contentHeight();
    const QString longText = QStringLiteral("Adding a row with a very long text");
    model.insertRow(0);
    model.setModelData(QPoint(0, 0), QSize(1, 1), longText);

    WAIT_UNTIL_POLISHED;

    QVERIFY(tableView->contentWidth() > initialContentWidth);
    QVERIFY(tableView->contentHeight() > initialContentHeight);
}

void tst_QQuickTableView::checkPageFlicking()
{
    // Check that we rebuild the table instead of refilling edges, if the viewport moves
    // more than a page (the size of TableView).
    LOAD_TABLEVIEW("plaintableview.qml");

    const int cellWidth = 100;
    const int cellHeight = 50;
    auto model = TestModelAsVariant(10000, 10000);
    const auto &loadedRows = tableViewPrivate->loadedRows;
    const auto &loadedColumns = tableViewPrivate->loadedColumns;

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // Sanity check startup table
    QCOMPARE(tableViewPrivate->topRow(), 0);
    QCOMPARE(tableViewPrivate->leftColumn(), 0);
    QCOMPARE(loadedRows.count(), tableView->height() / cellHeight);
    QCOMPARE(loadedColumns.count(), tableView->width() / cellWidth);

    // Since all cells have the same size, the average row/column
    // size found by TableView should be exactly equal to this.
    QCOMPARE(tableViewPrivate->averageEdgeSize.width(), cellWidth);
    QCOMPARE(tableViewPrivate->averageEdgeSize.height(), cellHeight);

    QCOMPARE(tableViewPrivate->scheduledRebuildOptions, QQuickTableViewPrivate::RebuildOption::None);

    // Flick 5000 columns to the right, and check that this triggers a
    // rebuild, and that we end up at the expected top-left.
    const int flickToColumn = 5000;
    const qreal columnSpacing = tableView->columnSpacing();
    const qreal flickToColumnInPixels = ((cellWidth + columnSpacing) * flickToColumn) - columnSpacing;
    tableView->setContentX(flickToColumnInPixels);

    QVERIFY(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::ViewportOnly);
    QVERIFY(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftColumn);
    QVERIFY(!(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftRow));

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->topRow(), 0);
    QCOMPARE(tableViewPrivate->leftColumn(), flickToColumn);
    QCOMPARE(loadedColumns.count(), tableView->width() / cellWidth);
    QCOMPARE(loadedRows.count(), tableView->height() / cellHeight);

    // Flick 5000 rows down as well. Since flicking down should only calculate a new row (but
    // keep the current column), we deliberatly change the average width to check that it's
    // actually ignored by the rebuild, and that the column stays the same.
    tableViewPrivate->averageEdgeSize.rwidth() /= 2;

    const int flickToRow = 5000;
    const qreal rowSpacing = tableView->rowSpacing();
    const qreal flickToRowInPixels = ((cellHeight + rowSpacing) * flickToRow) - rowSpacing;
    tableView->setContentY(flickToRowInPixels);

    QVERIFY(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::ViewportOnly);
    QVERIFY(!(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftColumn));
    QVERIFY(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftRow);

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->topRow(), flickToColumn);
    QCOMPARE(tableViewPrivate->leftColumn(), flickToRow);
    QCOMPARE(loadedRows.count(), tableView->height() / cellHeight);
    QCOMPARE(loadedColumns.count(), tableView->width() / cellWidth);
}

void tst_QQuickTableView::checkExplicitContentWidthAndHeight()
{
    // Check that you can set a custom contentWidth/Height, and that
    // TableView doesn't override it while loading more rows and columns.
    LOAD_TABLEVIEW("contentwidthheight.qml");

    tableView->setContentWidth(1000);
    tableView->setContentHeight(1000);
    QCOMPARE(tableView->contentWidth(), 1000);
    QCOMPARE(tableView->contentHeight(), 1000);

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;

    // Flick somewhere. It should not affect the contentWidth/Height
    tableView->setContentX(500);
    tableView->setContentY(500);
    QCOMPARE(tableView->contentWidth(), 1000);
    QCOMPARE(tableView->contentHeight(), 1000);
}

void tst_QQuickTableView::checkExtents_origin()
{
    // Check that if the beginning of the content view doesn't match the
    // actual size of the table, origin will be adjusted to make it fit.
    LOAD_TABLEVIEW("contentwidthheight.qml");

    const int rows = 10;
    const int columns = rows;
    const qreal columnWidth = 100;
    const qreal rowHeight = 100;
    const qreal actualTableSize = columns * columnWidth;

    // Set a content size that is far too large
    // compared to the size of the table.
    tableView->setContentWidth(actualTableSize * 2);
    tableView->setContentHeight(actualTableSize * 2);
    tableView->setRowSpacing(0);
    tableView->setColumnSpacing(0);
    tableView->setLeftMargin(0);
    tableView->setRightMargin(0);
    tableView->setTopMargin(0);
    tableView->setBottomMargin(0);

    auto model = TestModelAsVariant(rows, columns);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // Flick slowly to column 5 (to avoid rebuilds). Flick two columns at a
    // time to ensure that we create a gap before TableView gets a chance to
    // adjust endExtent first. This gap on the right side will make TableView
    // move the table to move to the edge. Because of this, the table will not
    // be aligned at the start of the content view when we next flick back again.
    // And this will cause origin to move.
    for (int x = 0; x <= 6; x += 2) {
        tableView->setContentX(x * columnWidth);
        tableView->setContentY(x * rowHeight);
    }

    // Check that the table has now been moved one column to the right
    // (One column because that's how far outside the table we ended up flicking above).
    QCOMPARE(tableViewPrivate->loadedTableOuterRect.right(), actualTableSize + columnWidth);

    // Flick back one column at a time so that TableView detects that the first
    // column is not at the origin before the "table move" logic kicks in. This
    // will make TableView adjust the origin.
    for (int x = 6; x >= 0; x -= 1) {
        tableView->setContentX(x * columnWidth);
        tableView->setContentY(x * rowHeight);
    }

    // The origin will be moved with the same offset that the table was
    // moved on the right side earlier, which is one column length.
    QCOMPARE(tableViewPrivate->origin.x(), columnWidth);
    QCOMPARE(tableViewPrivate->origin.y(), rowHeight);
}

void tst_QQuickTableView::checkExtents_endExtent()
{
    // Check that if we the content view size doesn't match the actual size
    // of the table, endExtent will be adjusted to make it fit (so that
    // e.g the the flicking will bounce to a stop at the edge of the table).
    LOAD_TABLEVIEW("contentwidthheight.qml");

    const int rows = 10;
    const int columns = rows;
    const qreal columnWidth = 100;
    const qreal rowHeight = 100;
    const qreal actualTableSize = columns * columnWidth;

    // Set a content size that is far too large
    // compared to the size of the table.
    tableView->setContentWidth(actualTableSize * 2);
    tableView->setContentHeight(actualTableSize * 2);
    tableView->setRowSpacing(0);
    tableView->setColumnSpacing(0);
    tableView->setLeftMargin(0);
    tableView->setRightMargin(0);
    tableView->setTopMargin(0);
    tableView->setBottomMargin(0);

    auto model = TestModelAsVariant(rows, columns);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // Flick slowly to column 5 (to avoid rebuilds). This will flick the table to
    // the last column in the model. But since there still is a lot space left in
    // the content view, endExtent will be set accordingly to compensate.
    for (int x = 1; x <= 5; x++)
        tableView->setContentX(x * columnWidth);
    QCOMPARE(tableViewPrivate->rightColumn(), columns - 1);
    qreal expectedEndExtentWidth = actualTableSize - tableView->contentWidth();
    QCOMPARE(tableViewPrivate->endExtent.width(), expectedEndExtentWidth);

    for (int y = 1; y <= 5; y++)
        tableView->setContentY(y * rowHeight);
    QCOMPARE(tableViewPrivate->bottomRow(), rows - 1);
    qreal expectedEndExtentHeight = actualTableSize - tableView->contentHeight();
    QCOMPARE(tableViewPrivate->endExtent.height(), expectedEndExtentHeight);
}

void tst_QQuickTableView::checkExtents_moveTableToEdge()
{
    // Check that if we the content view size doesn't match the actual
    // size of the table, and we fast-flick the viewport to outside
    // the table, we end up moving the table back into the viewport to
    // avoid any visual glitches.
    LOAD_TABLEVIEW("contentwidthheight.qml");

    const int rows = 10;
    const int columns = rows;
    const qreal columnWidth = 100;
    const qreal rowHeight = 100;
    const qreal actualTableSize = columns * columnWidth;

    // Set a content size that is far to large
    // compared to the size of the table.
    tableView->setContentWidth(actualTableSize * 2);
    tableView->setContentHeight(actualTableSize * 2);
    tableView->setRowSpacing(0);
    tableView->setColumnSpacing(0);
    tableView->setLeftMargin(0);
    tableView->setRightMargin(0);
    tableView->setTopMargin(0);
    tableView->setBottomMargin(0);

    auto model = TestModelAsVariant(rows, columns);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // Flick slowly to column 5 (to avoid rebuilds). Flick two columns at a
    // time to ensure that we create a gap before TableView gets a chance to
    // adjust endExtent first. This gap on the right side will make TableView
    // move the table to the edge (in addition to adjusting the extents, but that
    // will happen in a subsequent polish, and is not for this test verify).
    for (int x = 0; x <= 6; x += 2)
        tableView->setContentX(x * columnWidth);
    QCOMPARE(tableViewPrivate->rightColumn(), columns - 1);
    QCOMPARE(tableViewPrivate->loadedTableOuterRect, tableViewPrivate->viewportRect);

    for (int y = 0; y <= 6; y += 2)
        tableView->setContentY(y * rowHeight);
    QCOMPARE(tableViewPrivate->bottomRow(), rows - 1);
    QCOMPARE(tableViewPrivate->loadedTableOuterRect, tableViewPrivate->viewportRect);

    for (int x = 6; x >= 0; x -= 2)
        tableView->setContentX(x * columnWidth);
    QCOMPARE(tableViewPrivate->leftColumn(), 0);
    QCOMPARE(tableViewPrivate->loadedTableOuterRect, tableViewPrivate->viewportRect);

    for (int y = 6; y >= 0; y -= 2)
        tableView->setContentY(y * rowHeight);
    QCOMPARE(tableViewPrivate->topRow(), 0);
    QCOMPARE(tableViewPrivate->loadedTableOuterRect, tableViewPrivate->viewportRect);
}

void tst_QQuickTableView::checkContentXY()
{
    // Check that you can bind contentX and contentY to
    // e.g show the center of the table at start-up
    LOAD_TABLEVIEW("setcontentpos.qml");

    auto model = TestModelAsVariant(10, 10);
    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableView->width(), 400);
    QCOMPARE(tableView->height(), 400);
    QCOMPARE(tableView->contentWidth(), 1000);
    QCOMPARE(tableView->contentHeight(), 1000);

    // Check that the content item is positioned according
    // to the binding in the QML file (which will set the
    // viewport to be at the center of the table).
    const qreal expectedXY = (tableView->contentWidth() - tableView->width()) / 2;
    QCOMPARE(tableView->contentX(), expectedXY);
    QCOMPARE(tableView->contentY(), expectedXY);

    // Check that we end up at the correct top-left cell:
    const qreal delegateWidth = tableViewPrivate->loadedItems.values().first()->item->width();
    const int expectedCellXY = qCeil(expectedXY / delegateWidth);
    QCOMPARE(tableViewPrivate->leftColumn(), expectedCellXY);
    QCOMPARE(tableViewPrivate->topRow(), expectedCellXY);
}

void tst_QQuickTableView::noDelegate()
{
    // Check that you can skip setting a delegate without
    // it causing any problems (like crashing or asserting).
    // And then set a delegate, and do a quick check that the
    // view gets populated as expected.
    LOAD_TABLEVIEW("plaintableview.qml");

    const int rows = 5;
    const int columns = 5;
    auto model = TestModelAsVariant(columns, rows);
    tableView->setModel(model);

    // Start with no delegate, and check
    // that we end up with no items in the table.
    tableView->setDelegate(nullptr);

    WAIT_UNTIL_POLISHED;

    auto items = tableViewPrivate->loadedItems;
    QVERIFY(items.isEmpty());

    // Set a delegate, and check that we end
    // up with the expected number of items.
    auto delegate = view->rootObject()->property("delegate").value<QQmlComponent *>();
    QVERIFY(delegate);
    tableView->setDelegate(delegate);

    WAIT_UNTIL_POLISHED;

    items = tableViewPrivate->loadedItems;
    QCOMPARE(items.count(), rows * columns);

    // And then unset the delegate again, and check
    // that we end up with no items.
    tableView->setDelegate(nullptr);

    WAIT_UNTIL_POLISHED;

    items = tableViewPrivate->loadedItems;
    QVERIFY(items.isEmpty());
}

void tst_QQuickTableView::changeDelegateDuringUpdate()
{
    // Check that you can change the delegate (set it to null)
    // while the TableView is busy loading the table.
    LOAD_TABLEVIEW("changemodelordelegateduringupdate.qml");

    auto model = TestModelAsVariant(1, 1);
    tableView->setModel(model);
    view->rootObject()->setProperty("changeDelegate", true);

    WAIT_UNTIL_POLISHED;

    // We should no longer have a delegate, and no
    // items should therefore be loaded.
    QCOMPARE(tableView->delegate(), nullptr);
    QCOMPARE(tableViewPrivate->loadedItems.size(), 0);

    // Even if the delegate is missing, we still report
    // the correct size of the model
    QCOMPARE(tableView->rows(), 1);
    QCOMPARE(tableView->columns(), 1);
};

void tst_QQuickTableView::changeModelDuringUpdate()
{
    // Check that you can change the model (set it to null)
    // while the TableView is buzy loading the table.
    LOAD_TABLEVIEW("changemodelordelegateduringupdate.qml");

    auto model = TestModelAsVariant(1, 1);
    tableView->setModel(model);
    view->rootObject()->setProperty("changeModel", true);

    WAIT_UNTIL_POLISHED;

    // We should no longer have a model, and the no
    // items should therefore be loaded.
    QVERIFY(tableView->model().isNull());
    QCOMPARE(tableViewPrivate->loadedItems.size(), 0);

    // The empty model has no rows or columns
    QCOMPARE(tableView->rows(), 0);
    QCOMPARE(tableView->columns(), 0);
};

void tst_QQuickTableView::countDelegateItems_data()
{
    QTest::addColumn<QVariant>("model");
    QTest::addColumn<int>("count");

    QTest::newRow("QAIM 1x1") << TestModelAsVariant(1, 1) << 1;
    QTest::newRow("QAIM 2x1") << TestModelAsVariant(2, 1) << 2;
    QTest::newRow("QAIM 1x2") << TestModelAsVariant(1, 2) << 2;
    QTest::newRow("QAIM 2x2") << TestModelAsVariant(2, 2) << 4;
    QTest::newRow("QAIM 4x4") << TestModelAsVariant(4, 4) << 16;

    QTest::newRow("Number model 1") << QVariant::fromValue(1) << 1;
    QTest::newRow("Number model 4") << QVariant::fromValue(4) << 4;

    QTest::newRow("QStringList 1") << QVariant::fromValue(QStringList() << "one") << 1;
    QTest::newRow("QStringList 4") << QVariant::fromValue(QStringList() << "one" << "two" << "three" << "four") << 4;
}

void tst_QQuickTableView::countDelegateItems()
{
    // Assign different models of various sizes, and check that the number of
    // delegate items in the view matches the size of the model. Note that for
    // this test to be valid, all items must be within the visible area of the view.
    QFETCH(QVariant, model);
    QFETCH(int, count);
    LOAD_TABLEVIEW("plaintableview.qml");

    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;

    // Check that tableview internals contain the expected number of items
    auto const items = tableViewPrivate->loadedItems;
    QCOMPARE(items.count(), count);

    // Check that this also matches the items found in the view
    auto foundItems = findItems<QQuickItem>(tableView, kDelegateObjectName);
    QCOMPARE(foundItems.count(), count);
}

void tst_QQuickTableView::checkLayoutOfEqualSizedDelegateItems_data()
{
    QTest::addColumn<QVariant>("model");
    QTest::addColumn<QSize>("tableSize");
    QTest::addColumn<QSizeF>("spacing");
    QTest::addColumn<QMarginsF>("margins");

    // Check spacing together with different table setups
    QTest::newRow("QAIM 1x1 1,1") << TestModelAsVariant(1, 1) << QSize(1, 1) << QSizeF(1, 1) << QMarginsF(0, 0, 0, 0);
    QTest::newRow("QAIM 5x5 0,0") << TestModelAsVariant(5, 5) << QSize(5, 5) << QSizeF(0, 0) << QMarginsF(0, 0, 0, 0);
    QTest::newRow("QAIM 5x5 1,0") << TestModelAsVariant(5, 5) << QSize(5, 5) << QSizeF(1, 0) << QMarginsF(0, 0, 0, 0);
    QTest::newRow("QAIM 5x5 0,1") << TestModelAsVariant(5, 5) << QSize(5, 5) << QSizeF(0, 1) << QMarginsF(0, 0, 0, 0);

    // Check spacing together with margins
    QTest::newRow("QAIM 1x1 1,1 5555") << TestModelAsVariant(1, 1) << QSize(1, 1) << QSizeF(1, 1) << QMarginsF(5, 5, 5, 5);
    QTest::newRow("QAIM 4x4 0,0 3333") << TestModelAsVariant(4, 4) << QSize(4, 4) << QSizeF(0, 0) << QMarginsF(3, 3, 3, 3);
    QTest::newRow("QAIM 4x4 2,2 1234") << TestModelAsVariant(4, 4) << QSize(4, 4) << QSizeF(2, 2) << QMarginsF(1, 2, 3, 4);

    // Check "list" models
    QTest::newRow("NumberModel 1x4, 0000") << QVariant::fromValue(4) << QSize(1, 4) << QSizeF(1, 1) << QMarginsF(0, 0, 0, 0);
    QTest::newRow("QStringList 1x4, 0,0 1111") << QVariant::fromValue(QStringList() << "one" << "two" << "three" << "four")
                                               << QSize(1, 4) << QSizeF(0, 0) << QMarginsF(1, 1, 1, 1);
}

void tst_QQuickTableView::checkLayoutOfEqualSizedDelegateItems()
{
    // Check that the geometry of the delegate items are correct
    QFETCH(QVariant, model);
    QFETCH(QSize, tableSize);
    QFETCH(QSizeF, spacing);
    QFETCH(QMarginsF, margins);
    LOAD_TABLEVIEW("plaintableview.qml");

    const qreal expectedItemWidth = 100;
    const qreal expectedItemHeight = 50;
    const int expectedItemCount = tableSize.width() * tableSize.height();

    tableView->setModel(model);
    tableView->setRowSpacing(spacing.height());
    tableView->setColumnSpacing(spacing.width());

    // Setting margins on Flickable should not affect the layout of the
    // delegate items, since the margins is "transparent" to the TableView.
    tableView->setLeftMargin(margins.left());
    tableView->setTopMargin(margins.top());
    tableView->setRightMargin(margins.right());
    tableView->setBottomMargin(margins.bottom());

    WAIT_UNTIL_POLISHED;

    auto const items = tableViewPrivate->loadedItems;
    QVERIFY(!items.isEmpty());

    for (int i = 0; i < expectedItemCount; ++i) {
        const QQuickItem *item = items[i]->item;
        QVERIFY(item);
        QCOMPARE(item->parentItem(), tableView->contentItem());

        const QPoint cell = getContextRowAndColumn(item);
        qreal expectedX = cell.x() * (expectedItemWidth + spacing.width());
        qreal expectedY = cell.y() * (expectedItemHeight + spacing.height());
        QCOMPARE(item->x(), expectedX);
        QCOMPARE(item->y(), expectedY);
        QCOMPARE(item->z(), 1);
        QCOMPARE(item->width(), expectedItemWidth);
        QCOMPARE(item->height(), expectedItemHeight);
    }
}

void tst_QQuickTableView::checkFocusRemoved_data()
{
    QTest::addColumn<QString>("focusedItemProp");

    QTest::newRow("delegate root") << QStringLiteral("delegateRoot");
    QTest::newRow("delegate child") << QStringLiteral("delegateChild");
}

void tst_QQuickTableView::checkFocusRemoved()
{
    // Check that we clear the focus of a delegate item when
    // a child of the delegate item has focus, and the cell is
    // flicked out of view.
    QFETCH(QString, focusedItemProp);
    LOAD_TABLEVIEW("tableviewfocus.qml");

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    auto const item = tableViewPrivate->loadedTableItem(QPoint(0, 0))->item;
    auto const focusedItem = qvariant_cast<QQuickItem *>(item->property(focusedItemProp.toUtf8().data()));
    QVERIFY(focusedItem);
    QCOMPARE(tableView->hasActiveFocus(), false);
    QCOMPARE(focusedItem->hasActiveFocus(), false);

    focusedItem->forceActiveFocus();
    QCOMPARE(tableView->hasActiveFocus(), true);
    QCOMPARE(focusedItem->hasActiveFocus(), true);

    // Flick the focused cell out, and check that none of the
    // items in the table has focus (which means that the reused
    // item lost focus when it was flicked out). But the tableview
    // itself will maintain active focus.
    tableView->setContentX(500);
    QCOMPARE(tableView->hasActiveFocus(), true);
    for (auto fxItem : tableViewPrivate->loadedItems) {
        auto const focusedItem2 = qvariant_cast<QQuickItem *>(fxItem->item->property(focusedItemProp.toUtf8().data()));
        QCOMPARE(focusedItem2->hasActiveFocus(), false);
    }
}

void tst_QQuickTableView::fillTableViewButNothingMore_data()
{
    QTest::addColumn<QSizeF>("spacing");

    QTest::newRow("0 0,0 0") << QSizeF(0, 0);
    QTest::newRow("0 10,10 0") << QSizeF(10, 10);
    QTest::newRow("100 10,10 0") << QSizeF(10, 10);
    QTest::newRow("0 0,0 100") << QSizeF(0, 0);
    QTest::newRow("0 10,10 100") << QSizeF(10, 10);
    QTest::newRow("100 10,10 100") << QSizeF(10, 10);
}

void tst_QQuickTableView::fillTableViewButNothingMore()
{
    // Check that we end up filling the whole visible part of
    // the tableview with cells, but nothing more.
    QFETCH(QSizeF, spacing);
    LOAD_TABLEVIEW("plaintableview.qml");

    const int rows = 100;
    const int columns = 100;
    auto model = TestModelAsVariant(rows, columns);

    tableView->setModel(model);
    tableView->setRowSpacing(spacing.height());
    tableView->setColumnSpacing(spacing.width());

    WAIT_UNTIL_POLISHED;

    auto const topLeftFxItem = tableViewPrivate->loadedTableItem(QPoint(0, 0));
    auto const topLeftItem = topLeftFxItem->item;

    auto const bottomRightLoadedCell = QPoint(tableViewPrivate->rightColumn(), tableViewPrivate->bottomRow());
    auto const bottomRightFxItem = tableViewPrivate->loadedTableItem(bottomRightLoadedCell);
    auto const bottomRightItem = bottomRightFxItem->item;
    const QPoint bottomRightCell = getContextRowAndColumn(bottomRightItem.data());

    // Check that the right-most item is overlapping the right edge of the view
    QVERIFY(bottomRightItem->x() < tableView->width());
    QVERIFY(bottomRightItem->x() + bottomRightItem->width() >= tableView->width() - spacing.width());

    // Check that the actual number of columns matches what we expect
    qreal cellWidth = bottomRightItem->width() + spacing.width();
    int expectedColumns = qCeil(tableView->width() / cellWidth);
    int actualColumns = bottomRightCell.x() + 1;
    QCOMPARE(actualColumns, expectedColumns);

    // Check that the bottom-most item is overlapping the bottom edge of the view
    QVERIFY(bottomRightItem->y() < tableView->height());
    QVERIFY(bottomRightItem->y() + bottomRightItem->height() >= tableView->height() - spacing.height());

    // Check that the actual number of rows matches what we expect
    qreal cellHeight = bottomRightItem->height() + spacing.height();
    int expectedRows = qCeil(tableView->height() / cellHeight);
    int actualRows = bottomRightCell.y() + 1;
    QCOMPARE(actualRows, expectedRows);
}

void tst_QQuickTableView::checkInitialAttachedProperties_data()
{
    QTest::addColumn<QVariant>("model");

    QTest::newRow("QAIM") << TestModelAsVariant(4, 4);
    QTest::newRow("Number model") << QVariant::fromValue(4);
    QTest::newRow("QStringList") << QVariant::fromValue(QStringList() << "0" << "1" << "2" << "3");
}

void tst_QQuickTableView::checkInitialAttachedProperties()
{
    // Check that the context and attached properties inside
    // the delegate items are what we expect at start-up.
    QFETCH(QVariant, model);
    LOAD_TABLEVIEW("plaintableview.qml");

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems) {
        const int index = fxItem->index;
        const auto item = fxItem->item;
        const auto context = qmlContext(item.data());
        const QPoint cell = tableViewPrivate->cellAtModelIndex(index);
        const int contextIndex = context->contextProperty("index").toInt();
        const QPoint contextCell = getContextRowAndColumn(item.data());
        const QString contextModelData = context->contextProperty("modelData").toString();

        QCOMPARE(contextCell.y(), cell.y());
        QCOMPARE(contextCell.x(), cell.x());
        QCOMPARE(contextIndex, index);
        QCOMPARE(contextModelData, QStringLiteral("%1").arg(cell.y()));
        QCOMPARE(getAttachedObject(item)->view(), tableView);
    }
}

void tst_QQuickTableView::checkSpacingValues()
{
    LOAD_TABLEVIEW("tableviewdefaultspacing.qml");

    int rowCount = 9;
    int columnCount = 9;
    int delegateWidth = 15;
    int delegateHeight = 10;
    auto model = TestModelAsVariant(rowCount, columnCount);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // Default spacing : 0
    QCOMPARE(tableView->rowSpacing(), 0);
    QCOMPARE(tableView->columnSpacing(), 0);

    tableView->polish();
    WAIT_UNTIL_POLISHED;

    qreal expectedContentWidth = columnCount * (delegateWidth + tableView->columnSpacing()) - tableView->columnSpacing();
    qreal expectedContentHeight = rowCount * (delegateHeight + tableView->rowSpacing()) - tableView->rowSpacing();
    QCOMPARE(tableView->contentWidth(), expectedContentWidth);
    QCOMPARE(tableView->contentHeight(), expectedContentHeight);

    // Valid spacing assignment
    tableView->setRowSpacing(42);
    tableView->setColumnSpacing(12);
    QCOMPARE(tableView->rowSpacing(), 42);
    QCOMPARE(tableView->columnSpacing(), 12);

    tableView->polish();
    WAIT_UNTIL_POLISHED;

    expectedContentWidth = columnCount * (delegateWidth + tableView->columnSpacing()) - tableView->columnSpacing();
    expectedContentHeight = rowCount * (delegateHeight + tableView->rowSpacing()) - tableView->rowSpacing();
    QCOMPARE(tableView->contentWidth(), expectedContentWidth);
    QCOMPARE(tableView->contentHeight(), expectedContentHeight);

    // Negative spacing is allowed, and can be used to eliminate double edges
    // in the grid if the delegate is a rectangle with a border.
    tableView->setRowSpacing(-1);
    tableView->setColumnSpacing(-1);
    QCOMPARE(tableView->rowSpacing(), -1);
    QCOMPARE(tableView->columnSpacing(), -1);

    tableView->setRowSpacing(10);
    tableView->setColumnSpacing(10);
    // Invalid assignments (should ignore)
    tableView->setRowSpacing(INFINITY);
    tableView->setColumnSpacing(INFINITY);
    tableView->setRowSpacing(NAN);
    tableView->setColumnSpacing(NAN);
    QCOMPARE(tableView->rowSpacing(), 10);
    QCOMPARE(tableView->columnSpacing(), 10);
}

void tst_QQuickTableView::checkDelegateParent()
{
    // Check that TableView sets the delegate parent before
    // bindings are evaluated, so that the app can bind to it.
    LOAD_TABLEVIEW("plaintableview.qml");

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    QVERIFY(view->rootObject()->property("delegateParentSetBeforeCompleted").toBool());
}

void tst_QQuickTableView::flick_data()
{
    QTest::addColumn<QSizeF>("spacing");
    QTest::addColumn<QMarginsF>("margins");
    QTest::addColumn<bool>("reuseItems");

    QTest::newRow("s:0 m:0 reuse") << QSizeF(0, 0) << QMarginsF(0, 0, 0, 0) << true;
    QTest::newRow("s:5 m:0 reuse") << QSizeF(5, 5) << QMarginsF(0, 0, 0, 0) << true;
    QTest::newRow("s:0 m:20 reuse") << QSizeF(0, 0) << QMarginsF(20, 20, 20, 20) << true;
    QTest::newRow("s:5 m:20 reuse") << QSizeF(5, 5) << QMarginsF(20, 20, 20, 20) << true;
    QTest::newRow("s:0 m:0") << QSizeF(0, 0) << QMarginsF(0, 0, 0, 0) << false;
    QTest::newRow("s:5 m:0") << QSizeF(5, 5) << QMarginsF(0, 0, 0, 0) << false;
    QTest::newRow("s:0 m:20") << QSizeF(0, 0) << QMarginsF(20, 20, 20, 20) << false;
    QTest::newRow("s:5 m:20") << QSizeF(5, 5) << QMarginsF(20, 20, 20, 20) << false;
}

void tst_QQuickTableView::flick()
{
    // Check that if we end up with the correct start and end column/row as we flick around
    // with different table configurations.
    QFETCH(QSizeF, spacing);
    QFETCH(QMarginsF, margins);
    QFETCH(bool, reuseItems);
    LOAD_TABLEVIEW("plaintableview.qml");

    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    const int visualColumnCount = 4;
    const int visualRowCount = 4;
    const qreal cellWidth = delegateWidth + spacing.width();
    const qreal cellHeight = delegateHeight + spacing.height();
    auto model = TestModelAsVariant(100, 100);

    tableView->setModel(model);
    tableView->setRowSpacing(spacing.height());
    tableView->setColumnSpacing(spacing.width());
    tableView->setLeftMargin(margins.left());
    tableView->setTopMargin(margins.top());
    tableView->setRightMargin(margins.right());
    tableView->setBottomMargin(margins.bottom());
    tableView->setReuseItems(reuseItems);
    tableView->setWidth(margins.left() + (visualColumnCount * cellWidth) - spacing.width());
    tableView->setHeight(margins.top() + (visualRowCount * cellHeight) - spacing.height());

    WAIT_UNTIL_POLISHED;

    // Check the "simple" case if the cells never lands egde-to-edge with the viewport. For
    // that case we only accept that visible row/columns are loaded.
    qreal flickValues[] = {0.5, 1.5, 4.5, 20.5, 10.5, 3.5, 1.5, 0.5};

    for (qreal cellsToFlick : flickValues) {
        // Flick to the beginning of the cell
        tableView->setContentX(cellsToFlick * cellWidth);
        tableView->setContentY(cellsToFlick * cellHeight);
        tableView->polish();

        WAIT_UNTIL_POLISHED;

        const int expectedTableLeft = int(cellsToFlick - int((margins.left() + spacing.width()) / cellWidth));
        const int expectedTableTop = int(cellsToFlick - int((margins.top() + spacing.height()) / cellHeight));

        QCOMPARE(tableViewPrivate->leftColumn(), expectedTableLeft);
        QCOMPARE(tableViewPrivate->rightColumn(), expectedTableLeft + visualColumnCount);
        QCOMPARE(tableViewPrivate->topRow(), expectedTableTop);
        QCOMPARE(tableViewPrivate->bottomRow(), expectedTableTop + visualRowCount);
    }
}

void tst_QQuickTableView::flickOvershoot_data()
{
    QTest::addColumn<QSizeF>("spacing");
    QTest::addColumn<QMarginsF>("margins");
    QTest::addColumn<bool>("reuseItems");

    QTest::newRow("s:0 m:0 reuse") << QSizeF(0, 0) << QMarginsF(0, 0, 0, 0) << true;
    QTest::newRow("s:5 m:0 reuse") << QSizeF(5, 5) << QMarginsF(0, 0, 0, 0) << true;
    QTest::newRow("s:0 m:20 reuse") << QSizeF(0, 0) << QMarginsF(20, 20, 20, 20) << true;
    QTest::newRow("s:5 m:20 reuse") << QSizeF(5, 5) << QMarginsF(20, 20, 20, 20) << true;
    QTest::newRow("s:0 m:0") << QSizeF(0, 0) << QMarginsF(0, 0, 0, 0) << false;
    QTest::newRow("s:5 m:0") << QSizeF(5, 5) << QMarginsF(0, 0, 0, 0) << false;
    QTest::newRow("s:0 m:20") << QSizeF(0, 0) << QMarginsF(20, 20, 20, 20) << false;
    QTest::newRow("s:5 m:20") << QSizeF(5, 5) << QMarginsF(20, 20, 20, 20) << false;
}

void tst_QQuickTableView::flickOvershoot()
{
    // Flick the table completely out and then in again, and see
    // that we still contains the expected rows/columns
    // Note that TableView always keeps top-left item loaded, even
    // when everything is flicked out of view.
    QFETCH(QSizeF, spacing);
    QFETCH(QMarginsF, margins);
    QFETCH(bool, reuseItems);
    LOAD_TABLEVIEW("plaintableview.qml");

    const int rowCount = 5;
    const int columnCount = 5;
    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    const qreal cellWidth = delegateWidth + spacing.width();
    const qreal cellHeight = delegateHeight + spacing.height();
    const qreal tableWidth = margins.left() + margins.right() + (cellWidth * columnCount) - spacing.width();
    const qreal tableHeight = margins.top() + margins.bottom() + (cellHeight * rowCount) - spacing.height();
    const int outsideMargin = 10;
    auto model = TestModelAsVariant(rowCount, columnCount);

    tableView->setModel(model);
    tableView->setRowSpacing(spacing.height());
    tableView->setColumnSpacing(spacing.width());
    tableView->setLeftMargin(margins.left());
    tableView->setTopMargin(margins.top());
    tableView->setRightMargin(margins.right());
    tableView->setBottomMargin(margins.bottom());
    tableView->setReuseItems(reuseItems);
    tableView->setWidth(tableWidth - margins.right() - cellWidth / 2);
    tableView->setHeight(tableHeight - margins.bottom() - cellHeight / 2);

    WAIT_UNTIL_POLISHED;

    // Flick table out of view left
    tableView->setContentX(-tableView->width() - outsideMargin);
    tableView->setContentY(0);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->leftColumn(), 0);
    QCOMPARE(tableViewPrivate->rightColumn(), 0);
    QCOMPARE(tableViewPrivate->topRow(), 0);
    QCOMPARE(tableViewPrivate->bottomRow(), rowCount - 1);

    // Flick table out of view right
    tableView->setContentX(tableWidth + outsideMargin);
    tableView->setContentY(0);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->leftColumn(), columnCount - 1);
    QCOMPARE(tableViewPrivate->rightColumn(), columnCount - 1);
    QCOMPARE(tableViewPrivate->topRow(), 0);
    QCOMPARE(tableViewPrivate->bottomRow(), rowCount - 1);

    // Flick table out of view on top
    tableView->setContentX(0);
    tableView->setContentY(-tableView->height() - outsideMargin);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->leftColumn(), 0);
    QCOMPARE(tableViewPrivate->rightColumn(), columnCount - 1);
    QCOMPARE(tableViewPrivate->topRow(), 0);
    QCOMPARE(tableViewPrivate->bottomRow(), 0);

    // Flick table out of view at the bottom
    tableView->setContentX(0);
    tableView->setContentY(tableHeight + outsideMargin);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->leftColumn(), 0);
    QCOMPARE(tableViewPrivate->rightColumn(), columnCount - 1);
    QCOMPARE(tableViewPrivate->topRow(), rowCount - 1);
    QCOMPARE(tableViewPrivate->bottomRow(), rowCount - 1);

    // Flick table out of view left and top at the same time
    tableView->setContentX(-tableView->width() - outsideMargin);
    tableView->setContentY(-tableView->height() - outsideMargin);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->leftColumn(), 0);
    QCOMPARE(tableViewPrivate->rightColumn(), 0);
    QCOMPARE(tableViewPrivate->topRow(), 0);
    QCOMPARE(tableViewPrivate->bottomRow(), 0);

    // Flick table back to origo
    tableView->setContentX(0);
    tableView->setContentY(0);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->leftColumn(), 0);
    QCOMPARE(tableViewPrivate->rightColumn(), columnCount - 1);
    QCOMPARE(tableViewPrivate->topRow(), 0);
    QCOMPARE(tableViewPrivate->bottomRow(), rowCount - 1);

    // Flick table out of view right and bottom at the same time
    tableView->setContentX(tableWidth + outsideMargin);
    tableView->setContentY(tableHeight + outsideMargin);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->leftColumn(), columnCount - 1);
    QCOMPARE(tableViewPrivate->rightColumn(), columnCount - 1);
    QCOMPARE(tableViewPrivate->topRow(), rowCount - 1);
    QCOMPARE(tableViewPrivate->bottomRow(), rowCount - 1);

    // Flick table back to origo
    tableView->setContentX(0);
    tableView->setContentY(0);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->leftColumn(), 0);
    QCOMPARE(tableViewPrivate->rightColumn(), columnCount - 1);
    QCOMPARE(tableViewPrivate->topRow(), 0);
    QCOMPARE(tableViewPrivate->bottomRow(), rowCount - 1);
}

void tst_QQuickTableView::checkRowColumnCount()
{
    // If we flick several columns (rows) at the same time, check that we don't
    // end up with loading more delegate items into memory than necessary. We
    // should free up columns as we go before loading new ones.
    LOAD_TABLEVIEW("countingtableview.qml");

    const char *maxDelegateCountProp = "maxDelegateCount";
    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    auto model = TestModelAsVariant(100, 100);
    const auto &loadedRows = tableViewPrivate->loadedRows;
    const auto &loadedColumns = tableViewPrivate->loadedColumns;

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // We expect that the number of created items after start-up should match
    //the size of the visible table, pluss one extra preloaded row and column.
    const int qmlCountAfterInit = view->rootObject()->property(maxDelegateCountProp).toInt();
    const int expectedCount = (loadedColumns.count() + 1) * (loadedRows.count() + 1);
    QCOMPARE(qmlCountAfterInit, expectedCount);

    // This test will keep track of the maximum number of delegate items TableView
    // had to show at any point while flicking (in countingtableview.qml). Because
    // of the geometries chosen for TableView and the delegate, only complete columns
    // will be shown at start-up.
    QVERIFY(loadedRows.count() > loadedColumns.count());
    QCOMPARE(tableViewPrivate->loadedTableOuterRect.width(), tableView->width());
    QCOMPARE(tableViewPrivate->loadedTableOuterRect.height(), tableView->height());

    // Flick half an item to the left+up, to force one extra column and row to load before we
    // start. By doing so, we end up showing the maximum number of rows and columns that will
    // ever be shown in the view. This will make things less complicated below, when checking
    // how many items that end up visible while flicking.
    tableView->setContentX(delegateWidth / 2);
    tableView->setContentY(delegateHeight / 2);
    const int qmlCountAfterFirstFlick = view->rootObject()->property(maxDelegateCountProp).toInt();

    // Flick a long distance right
    tableView->setContentX(tableView->width() * 2);

    const int qmlCountAfterLongFlick = view->rootObject()->property(maxDelegateCountProp).toInt();
    QCOMPARE(qmlCountAfterLongFlick, qmlCountAfterFirstFlick);

    // Flick a long distance down
    tableView->setContentX(tableView->height() * 2);

    const int qmlCountAfterDownFlick = view->rootObject()->property(maxDelegateCountProp).toInt();
    QCOMPARE(qmlCountAfterDownFlick, qmlCountAfterFirstFlick);

    // Flick a long distance left
    tableView->setContentX(0);

    const int qmlCountAfterLeftFlick = view->rootObject()->property(maxDelegateCountProp).toInt();
    QCOMPARE(qmlCountAfterLeftFlick, qmlCountAfterFirstFlick);

    // Flick a long distance up
    tableView->setContentY(0);

    const int qmlCountAfterUpFlick = view->rootObject()->property(maxDelegateCountProp).toInt();
    QCOMPARE(qmlCountAfterUpFlick, qmlCountAfterFirstFlick);
}

void tst_QQuickTableView::modelSignals()
{
    LOAD_TABLEVIEW("plaintableview.qml");

    TestModel model(1, 1);
    tableView->setModel(QVariant::fromValue(&model));
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 1);
    QCOMPARE(tableView->columns(), 1);

    QVERIFY(model.insertRows(0, 1));
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 2);
    QCOMPARE(tableView->columns(), 1);

    QVERIFY(model.removeRows(1, 1));
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 1);
    QCOMPARE(tableView->columns(), 1);

    model.insertColumns(1, 1);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 1);
    QCOMPARE(tableView->columns(), 2);

    model.removeColumns(1, 1);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 1);
    QCOMPARE(tableView->columns(), 1);

    model.setRowCount(10);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 10);
    QCOMPARE(tableView->columns(), 1);

    model.setColumnCount(10);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 10);
    QCOMPARE(tableView->columns(), 10);

    model.setRowCount(0);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 0);
    QCOMPARE(tableView->columns(), 10);

    model.setColumnCount(1);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 0);
    QCOMPARE(tableView->columns(), 1);

    model.setRowCount(10);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 10);
    QCOMPARE(tableView->columns(), 1);

    model.setColumnCount(10);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 10);
    QCOMPARE(tableView->columns(), 10);

    model.clear();
    model.setColumnCount(1);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->rows(), 0);
    QCOMPARE(tableView->columns(), 1);
}

void tst_QQuickTableView::checkModelSignalsUpdateLayout()
{
    // Check that if the model rearranges rows and emit the
    // 'layoutChanged' signal, TableView will be updated correctly.
    LOAD_TABLEVIEW("plaintableview.qml");

    TestModel model(0, 1);
    tableView->setModel(QVariant::fromValue(&model));
    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableView->rows(), 0);
    QCOMPARE(tableView->columns(), 1);

    QString modelRow1Text = QStringLiteral("firstRow");
    QString modelRow2Text = QStringLiteral("secondRow");
    model.insertRow(0);
    model.insertRow(0);
    model.setModelData(QPoint(0, 0), QSize(1, 1), modelRow1Text);
    model.setModelData(QPoint(0, 1), QSize(1, 1), modelRow2Text);
    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableView->rows(), 2);
    QCOMPARE(tableView->columns(), 1);

    QString delegate1text = tableViewPrivate->loadedTableItem(QPoint(0, 0))->item->property("modelDataBinding").toString();
    QString delegate2text = tableViewPrivate->loadedTableItem(QPoint(0, 1))->item->property("modelDataBinding").toString();
    QCOMPARE(delegate1text, modelRow1Text);
    QCOMPARE(delegate2text, modelRow2Text);

    model.swapRows(0, 1);
    WAIT_UNTIL_POLISHED;

    delegate1text = tableViewPrivate->loadedTableItem(QPoint(0, 0))->item->property("modelDataBinding").toString();
    delegate2text = tableViewPrivate->loadedTableItem(QPoint(0, 1))->item->property("modelDataBinding").toString();
    QCOMPARE(delegate1text, modelRow2Text);
    QCOMPARE(delegate2text, modelRow1Text);
}

void tst_QQuickTableView::dataChangedSignal()
{
    // Check that bindings to the model inside a delegate gets updated
    // when the model item they bind to changes.
    LOAD_TABLEVIEW("plaintableview.qml");

    const QString prefix(QStringLiteral("changed"));

    TestModel model(10, 10);
    tableView->setModel(QVariant::fromValue(&model));

    WAIT_UNTIL_POLISHED;

    for (auto fxItem : tableViewPrivate->loadedItems) {
        const auto item = tableViewPrivate->loadedTableItem(fxItem->cell)->item;
        const QString modelDataBindingProperty = item->property(kModelDataBindingProp).toString();
        QString expectedModelData = QString::number(fxItem->cell.y());
        QCOMPARE(modelDataBindingProperty, expectedModelData);
    }

    // Change one cell in the model
    model.setModelData(QPoint(0, 0), QSize(1, 1), prefix);

    for (auto fxItem : tableViewPrivate->loadedItems) {
        const QPoint cell = fxItem->cell;
        const auto modelIndex = model.index(cell.y(), cell.x());
        QString expectedModelData = model.data(modelIndex, Qt::DisplayRole).toString();

        const auto item = tableViewPrivate->loadedTableItem(fxItem->cell)->item;
        const QString modelDataBindingProperty = item->property(kModelDataBindingProp).toString();

        QCOMPARE(modelDataBindingProperty, expectedModelData);
    }

    // Change four cells in one go
    model.setModelData(QPoint(1, 0), QSize(2, 2), prefix);

    for (auto fxItem : tableViewPrivate->loadedItems) {
        const QPoint cell = fxItem->cell;
        const auto modelIndex = model.index(cell.y(), cell.x());
        QString expectedModelData = model.data(modelIndex, Qt::DisplayRole).toString();

        const auto item = tableViewPrivate->loadedTableItem(fxItem->cell)->item;
        const QString modelDataBindingProperty = item->property(kModelDataBindingProp).toString();

        QCOMPARE(modelDataBindingProperty, expectedModelData);
    }
}

void tst_QQuickTableView::checkThatPoolIsDrainedWhenReuseIsFalse()
{
    // Check that the reuse pool is drained
    // immediately when setting reuseItems to false.
    LOAD_TABLEVIEW("countingtableview.qml");

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // The pool should now contain preloaded items
    QVERIFY(tableViewPrivate->tableModel->poolSize() > 0);
    tableView->setReuseItems(false);
    // The pool should now be empty
    QCOMPARE(tableViewPrivate->tableModel->poolSize(), 0);
}

void tst_QQuickTableView::checkIfDelegatesAreReused_data()
{
    QTest::addColumn<bool>("reuseItems");

    QTest::newRow("reuse = true") << true;
    QTest::newRow("reuse = false") << false;
}

void tst_QQuickTableView::checkIfDelegatesAreReused()
{
    // Check that we end up reusing delegate items while flicking if
    // TableView has reuseItems set to true, but otherwise not.
    QFETCH(bool, reuseItems);
    LOAD_TABLEVIEW("countingtableview.qml");

    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    const int pageFlickCount = 4;

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);
    tableView->setReuseItems(reuseItems);

    WAIT_UNTIL_POLISHED;

    // Flick half an item to the side, to force one extra row and column to load before we start.
    // This will make things less complicated below, when checking how many times the items
    // have been reused (all items will then report the same number).
    tableView->setContentX(delegateWidth / 2);
    tableView->setContentY(delegateHeight / 2);
    QCOMPARE(tableViewPrivate->tableModel->poolSize(), 0);

    // Some items have already been pooled and reused after we moved the content view, because
    // we preload one extra row and column at start-up. So reset the count-properties back to 0
    // before we continue.
    for (auto fxItem : tableViewPrivate->loadedItems) {
        fxItem->item->setProperty("pooledCount", 0);
        fxItem->item->setProperty("reusedCount", 0);
    }

    const int visibleColumnCount = tableViewPrivate->loadedColumns.count();
    const int visibleRowCount = tableViewPrivate->loadedRows.count();
    const int delegateCountAfterInit = view->rootObject()->property(kDelegatesCreatedCountProp).toInt();

    for (int column = 1; column <= (visibleColumnCount * pageFlickCount); ++column) {
        // Flick columns to the left (and add one pixel to ensure the left column is completely out)
        tableView->setContentX((delegateWidth * column) + 1);
        // Check that the number of delegate items created so far is what we expect.
        const int delegatesCreatedCount = view->rootObject()->property(kDelegatesCreatedCountProp).toInt();
        int expectedCount = delegateCountAfterInit + (reuseItems ? 0 : visibleRowCount * column);
        QCOMPARE(delegatesCreatedCount, expectedCount);
    }

    // Check that each delegate item has been reused as many times
    // as we have flicked pages (if reuse is enabled).
    for (auto fxItem : tableViewPrivate->loadedItems) {
        int pooledCount = fxItem->item->property("pooledCount").toInt();
        int reusedCount = fxItem->item->property("reusedCount").toInt();
        if (reuseItems) {
            QCOMPARE(pooledCount, pageFlickCount);
            QCOMPARE(reusedCount, pageFlickCount);
        } else {
            QCOMPARE(pooledCount, 0);
            QCOMPARE(reusedCount, 0);
        }
    }
}

void tst_QQuickTableView::checkIfDelegatesAreReusedAsymmetricTableSize()
{
    // Check that we end up reusing all delegate items while flicking, also if the table contain
    // more columns than rows. In that case, if we flick out a whole row, we'll move a lot of
    // items into the pool. And if we then start flicking in columns, we'll only reuse a few of
    // them for each column. Still, we don't want the pool to release the superfluous items after
    // each load, since they are still in circulation and will be needed once we flick in a new
    // row at the end of the test.
    LOAD_TABLEVIEW("countingtableview.qml");

    const int columnCount = 20;
    const int rowCount = 2;
    const qreal delegateWidth = tableView->width() / columnCount;
    const qreal delegateHeight = (tableView->height() / rowCount) + 10;

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    // Let the height of each row be much bigger than the width of each column.
    view->rootObject()->setProperty("delegateWidth", delegateWidth);
    view->rootObject()->setProperty("delegateHeight", delegateHeight);

    WAIT_UNTIL_POLISHED;

    auto initialTopLeftItem = tableViewPrivate->loadedTableItem(QPoint(0, 0))->item;
    QVERIFY(initialTopLeftItem);
    int pooledCount = initialTopLeftItem->property("pooledCount").toInt();
    int reusedCount = initialTopLeftItem->property("reusedCount").toInt();
    QCOMPARE(pooledCount, 0);
    QCOMPARE(reusedCount, 0);

    // Flick half an item left+down, to force one extra row and column to load. By doing
    // so, we force the maximum number of rows and columns to show before we start the test.
    // This will make things less complicated below, when checking how many
    // times the items have been reused (all items will then report the same number).
    tableView->setContentX(delegateWidth * 0.5);
    tableView->setContentY(delegateHeight * 0.5);

    // Since we have flicked half a delegate to the left, the number of visible
    // columns is now one more than the column count were when we started the test.
    const int visibleColumnCount = tableViewPrivate->loadedColumns.count();
    QCOMPARE(visibleColumnCount, columnCount + 1);

    // We expect no items to have been pooled so far
    pooledCount = initialTopLeftItem->property("pooledCount").toInt();
    reusedCount = initialTopLeftItem->property("reusedCount").toInt();
    QCOMPARE(pooledCount, 0);
    QCOMPARE(reusedCount, 0);
    QCOMPARE(tableViewPrivate->tableModel->poolSize(), 0);

    // Flick one row out of view. This will move one whole row of items into the
    // pool without reusing them, since no new row is exposed at the bottom.
    tableView->setContentY(delegateHeight + 1);
    pooledCount = initialTopLeftItem->property("pooledCount").toInt();
    reusedCount = initialTopLeftItem->property("reusedCount").toInt();
    QCOMPARE(pooledCount, 1);
    QCOMPARE(reusedCount, 0);
    QCOMPARE(tableViewPrivate->tableModel->poolSize(), visibleColumnCount);

    const int delegateCountAfterInit = view->rootObject()->property(kDelegatesCreatedCountProp).toInt();

    // Start flicking in a lot of columns, and check that the created count stays the same
    for (int column = 1; column <= 10; ++column) {
        tableView->setContentX((delegateWidth * column) + 10);
        const int delegatesCreatedCount = view->rootObject()->property(kDelegatesCreatedCountProp).toInt();
        // Since we reuse items while flicking, the created count should stay the same
        QCOMPARE(delegatesCreatedCount, delegateCountAfterInit);
        // Since we flick out just as many columns as we flick in, the pool size should stay the same
        QCOMPARE(tableViewPrivate->tableModel->poolSize(), visibleColumnCount);
    }

    // Finally, flick one row back into view (but without flicking so far that we push the third
    // row out and into the pool). The pool should still contain the exact amount of items that
    // we had after we flicked the first row out. And this should be exactly the amount of items
    // needed to load the row back again. And this also means that the pool count should then return
    // back to 0.
    tableView->setContentY(delegateHeight - 1);
    const int delegatesCreatedCount = view->rootObject()->property(kDelegatesCreatedCountProp).toInt();
    QCOMPARE(delegatesCreatedCount, delegateCountAfterInit);
    QCOMPARE(tableViewPrivate->tableModel->poolSize(), 0);
}

void tst_QQuickTableView::checkContextProperties_data()
{
    QTest::addColumn<QVariant>("model");
    QTest::addColumn<bool>("reuseItems");

    auto stringList = QStringList();
    for (int i = 0; i < 100; ++i)
        stringList.append(QString::number(i));

    QTest::newRow("QAIM, reuse=false") << TestModelAsVariant(100, 100) << false;
    QTest::newRow("QAIM, reuse=true") << TestModelAsVariant(100, 100) << true;
    QTest::newRow("Number model, reuse=false") << QVariant::fromValue(100) << false;
    QTest::newRow("Number model, reuse=true") << QVariant::fromValue(100) << true;
    QTest::newRow("QStringList, reuse=false") << QVariant::fromValue(stringList) << false;
    QTest::newRow("QStringList, reuse=true") << QVariant::fromValue(stringList) << true;
}

void tst_QQuickTableView::checkContextProperties()
{
    // Check that the context properties of the delegate items
    // are what we expect while flicking, with or without item recycling.
    QFETCH(QVariant, model);
    QFETCH(bool, reuseItems);
    LOAD_TABLEVIEW("countingtableview.qml");

    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    const int rowCount = 100;
    const int pageFlickCount = 3;

    tableView->setModel(model);
    tableView->setReuseItems(reuseItems);

    WAIT_UNTIL_POLISHED;

    const int visibleRowCount = qMin(tableView->rows(), qCeil(tableView->height() / delegateHeight));
    const int visibleColumnCount = qMin(tableView->columns(), qCeil(tableView->width() / delegateWidth));

    for (int row = 1; row <= (visibleRowCount * pageFlickCount); ++row) {
        // Flick rows up
        tableView->setContentY((delegateHeight * row) + (delegateHeight / 2));
        tableView->polish();

        WAIT_UNTIL_POLISHED;

        for (int col = 0; col < visibleColumnCount; ++col) {
            const auto item = tableViewPrivate->loadedTableItem(QPoint(col, row))->item;
            const auto context = qmlContext(item.data());
            const int contextIndex = context->contextProperty("index").toInt();
            const int contextRow = context->contextProperty("row").toInt();
            const int contextColumn = context->contextProperty("column").toInt();
            const QString contextModelData = context->contextProperty("modelData").toString();

            QCOMPARE(contextIndex, row + (col * rowCount));
            QCOMPARE(contextRow, row);
            QCOMPARE(contextColumn, col);
            QCOMPARE(contextModelData, QStringLiteral("%1").arg(row));
        }
    }
}

void tst_QQuickTableView::checkContextPropertiesQQmlListProperyModel_data()
{
    QTest::addColumn<bool>("reuseItems");

    QTest::newRow("reuse=false") << false;
    QTest::newRow("reuse=true") << true;
}

void tst_QQuickTableView::checkContextPropertiesQQmlListProperyModel()
{
    // Check that the context properties of the delegate items
    // are what we expect while flicking, with or without item recycling.
    // This test hard-codes the model to be a QQmlListPropertyModel from
    // within the qml file.
    QFETCH(bool, reuseItems);
    LOAD_TABLEVIEW("qqmllistpropertymodel.qml");

    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    const int rowCount = 100;
    const int pageFlickCount = 3;

    tableView->setReuseItems(reuseItems);
    tableView->polish();

    WAIT_UNTIL_POLISHED;

    const int visibleRowCount = qMin(tableView->rows(), qCeil(tableView->height() / delegateHeight));
    const int visibleColumnCount = qMin(tableView->columns(), qCeil(tableView->width() / delegateWidth));

    for (int row = 1; row <= (visibleRowCount * pageFlickCount); ++row) {
        // Flick rows up
        tableView->setContentY((delegateHeight * row) + (delegateHeight / 2));
        tableView->polish();

        WAIT_UNTIL_POLISHED;

        for (int col = 0; col < visibleColumnCount; ++col) {
            const auto item = tableViewPrivate->loadedTableItem(QPoint(col, row))->item;
            const auto context = qmlContext(item.data());
            const int contextIndex = context->contextProperty("index").toInt();
            const int contextRow = context->contextProperty("row").toInt();
            const int contextColumn = context->contextProperty("column").toInt();
            const QObject *contextModelData = qvariant_cast<QObject *>(context->contextProperty("modelData"));
            const QString modelDataProperty = contextModelData->property("someCustomProperty").toString();

            QCOMPARE(contextIndex, row + (col * rowCount));
            QCOMPARE(contextRow, row);
            QCOMPARE(contextColumn, col);
            QCOMPARE(modelDataProperty, QStringLiteral("%1").arg(row));
        }
    }
}

void tst_QQuickTableView::checkRowAndColumnChangedButNotIndex()
{
    // Check that context row and column changes even if the index stays the
    // same when the item is reused. This can happen in rare cases if the item
    // is first used at e.g (row 1, col 0), but then reused at (row 0, col 1)
    // while the model has changed row count in-between.
    LOAD_TABLEVIEW("checkrowandcolumnnotchanged.qml");

    TestModel model(2, 1);
    tableView->setModel(QVariant::fromValue(&model));

    WAIT_UNTIL_POLISHED;

    model.removeRow(1);
    model.insertColumn(1);
    tableView->forceLayout();

    const auto item = tableViewPrivate->loadedTableItem(QPoint(1, 0))->item;
    const auto context = qmlContext(item.data());
    const int contextIndex = context->contextProperty("index").toInt();
    const int contextRow = context->contextProperty("row").toInt();
    const int contextColumn = context->contextProperty("column").toInt();

    QCOMPARE(contextIndex, 1);
    QCOMPARE(contextRow, 0);
    QCOMPARE(contextColumn, 1);
}

void tst_QQuickTableView::checkThatWeAlwaysEmitChangedUponItemReused()
{
    // Check that we always emit changes to index when we reuse an item, even
    // if it doesn't change. This is needed since the model can have changed
    // row or column count while the item was in the pool, which means that
    // any data referred to by the index property inside the delegate
    // will change too. So we need to refresh any bindings to index.
    // QTBUG-79209
    LOAD_TABLEVIEW("checkalwaysemit.qml");

    TestModel model(1, 1);
    tableView->setModel(QVariant::fromValue(&model));
    model.setModelData(QPoint(0, 0), QSize(1, 1), "old value");

    WAIT_UNTIL_POLISHED;

    const auto reuseItem = tableViewPrivate->loadedTableItem(QPoint(0, 0))->item;
    const auto context = qmlContext(reuseItem.data());

    // Remove the cell/row that has "old value" as model data, and
    // add a new one right after. The new cell will have the same
    // index, but with no model data assigned.
    // This change will not be detected by items in the pool. But since
    // we emit indexChanged when the item is reused, it will be updated then.
    model.removeRow(0);
    model.insertRow(0);

    WAIT_UNTIL_POLISHED;

    QCOMPARE(context->contextProperty("index").toInt(), 0);
    QCOMPARE(context->contextProperty("row").toInt(), 0);
    QCOMPARE(context->contextProperty("column").toInt(), 0);
    QCOMPARE(context->contextProperty("modelDataFromIndex").toString(), "");
}

void tst_QQuickTableView::checkChangingModelFromDelegate()
{
    // Check that we don't restart a rebuild of the table
    // while we're in the middle of rebuilding it from before
    LOAD_TABLEVIEW("changemodelfromdelegate.qml");

    // Set addRowFromDelegate. This will trigger the QML code to add a new
    // row and call forceLayout(). When TableView instantiates the first
    // delegate in the new row, the Component.onCompleted handler will try to
    // add a new row. But since we're currently rebuilding, this should be
    // scheduled for later.
    view->rootObject()->setProperty("addRowFromDelegate", true);

    // We now expect two rows in the table, one more than initially
    QCOMPARE(tableViewPrivate->tableSize.height(), 2);
    QCOMPARE(tableViewPrivate->loadedRows.count(), 2);

    // And since the QML code tried to add another row as well, we
    // expect rebuildScheduled to be true, and a polish event to be pending.
    QVERIFY(tableViewPrivate->scheduledRebuildOptions);
    QCOMPARE(tableViewPrivate->polishScheduled, true);
    WAIT_UNTIL_POLISHED;

    // After handling the polish event, we expect also the third row to now be added
    QCOMPARE(tableViewPrivate->tableSize.height(), 3);
    QCOMPARE(tableViewPrivate->loadedRows.count(), 3);
}

void tst_QQuickTableView::checkRebuildViewportOnly()
{
    // Check that we only rebuild from the current top-left cell
    // when you add or remove rows and columns. There should be
    // no need to do a rebuild from scratch in such cases.
    LOAD_TABLEVIEW("countingtableview.qml");

    const char *propName = "delegatesCreatedCount";
    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;

    TestModel model(100, 100);
    tableView->setModel(QVariant::fromValue(&model));

    WAIT_UNTIL_POLISHED;

    // Flick to row/column 50, 50
    tableView->setContentX(delegateWidth * 50);
    tableView->setContentY(delegateHeight * 50);

    // Set reuse items to false, just to make it easier to
    // check the number of items created during a rebuild.
    tableView->setReuseItems(false);
    const int itemCountBeforeRebuild = tableViewPrivate->loadedItems.count();

    // Since all cells have the same size, we expect that we end up creating
    // the same amount of items that were already showing before, even after
    // adding or removing rows and columns.
    view->rootObject()->setProperty(propName, 0);
    model.insertRow(51);
    WAIT_UNTIL_POLISHED;
    int countAfterRebuild = view->rootObject()->property(propName).toInt();
    QCOMPARE(countAfterRebuild, itemCountBeforeRebuild);

    view->rootObject()->setProperty(propName, 0);
    model.removeRow(51);
    WAIT_UNTIL_POLISHED;
    countAfterRebuild = view->rootObject()->property(propName).toInt();
    QCOMPARE(countAfterRebuild, itemCountBeforeRebuild);

    view->rootObject()->setProperty(propName, 0);
    model.insertColumn(51);
    WAIT_UNTIL_POLISHED;
    countAfterRebuild = view->rootObject()->property(propName).toInt();
    QCOMPARE(countAfterRebuild, itemCountBeforeRebuild);

    view->rootObject()->setProperty(propName, 0);
    model.removeColumn(51);
    WAIT_UNTIL_POLISHED;
    countAfterRebuild = view->rootObject()->property(propName).toInt();
    QCOMPARE(countAfterRebuild, itemCountBeforeRebuild);
}

void tst_QQuickTableView::useDelegateChooserWithoutDefault()
{
    // Check that the application issues a warning (but doesn't e.g
    // crash) if the delegate chooser doesn't cover all cells
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*failed"));
    LOAD_TABLEVIEW("usechooserwithoutdefault.qml");
    auto model = TestModelAsVariant(2, 1);
    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;
};

void tst_QQuickTableView::checkTableviewInsideAsyncLoader()
{
    // Check that you can put a TableView inside an async Loader, and
    // that the delegate items are created before the loader is ready.
    LOAD_TABLEVIEW_ASYNC("asyncplain.qml");

    // At this point the Loader has finished
    QCOMPARE(loader->status(), QQuickLoader::Ready);

    // Check that TableView has finished building
    QVERIFY(!tableViewPrivate->scheduledRebuildOptions);
    QCOMPARE(tableViewPrivate->rebuildState, QQuickTableViewPrivate::RebuildState::Done);

    // Check that all expected delegate items have been loaded
    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    int expectedColumns = qCeil(tableView->width() / delegateWidth);
    int expectedRows = qCeil(tableView->height() / delegateHeight);
    QCOMPARE(tableViewPrivate->loadedColumns.count(), expectedColumns);
    QCOMPARE(tableViewPrivate->loadedRows.count(), expectedRows);

    // Check that the loader was still in a loading state while TableView was creating
    // delegate items. If we delayed creating delegate items until we got the first
    // updatePolish() callback in QQuickTableView, this would not be the case.
    auto statusWhenDelegate0_0Completed = qvariant_cast<QQuickLoader::Status>(
                loader->item()->property("statusWhenDelegate0_0Created"));
    auto statusWhenDelegate5_5Completed = qvariant_cast<QQuickLoader::Status>(
                loader->item()->property("statusWhenDelegate5_5Created"));
    QCOMPARE(statusWhenDelegate0_0Completed, QQuickLoader::Loading);
    QCOMPARE(statusWhenDelegate5_5Completed, QQuickLoader::Loading);

    // Check that TableView had a valid geometry when we started to build. If the build
    // was started too early (e.g upon QQuickTableView::componentComplete), width and
    // height would still be 0 since the bindings would not have been evaluated yet.
    qreal width = loader->item()->property("tableViewWidthWhileBuilding").toReal();
    qreal height = loader->item()->property("tableViewHeightWhileBuilding").toReal();
    QVERIFY(width > 0);
    QVERIFY(height > 0);
};

#define INT_LIST(indices) QVariant::fromValue(QList<int>() << indices)

void tst_QQuickTableView::hideRowsAndColumns_data()
{
    QTest::addColumn<QVariant>("rowsToHide");
    QTest::addColumn<QVariant>("columnsToHide");

    const auto emptyList = QVariant::fromValue(QList<int>());

    // Hide rows
    QTest::newRow("first") << INT_LIST(0) << emptyList;
    QTest::newRow("middle 1") << INT_LIST(1) << emptyList;
    QTest::newRow("middle 3") << INT_LIST(3) << emptyList;
    QTest::newRow("last") << INT_LIST(4) << emptyList;

    QTest::newRow("subsequent 0,1") << INT_LIST(0 << 1) << emptyList;
    QTest::newRow("subsequent 1,2") << INT_LIST(1 << 2) << emptyList;
    QTest::newRow("subsequent 3,4") << INT_LIST(3 << 4) << emptyList;

    QTest::newRow("all but first") << INT_LIST(1 << 2 << 3 << 4) << emptyList;
    QTest::newRow("all but last") << INT_LIST(0 << 1 << 2 << 3) << emptyList;
    QTest::newRow("all but middle") << INT_LIST(0 << 1 << 3 << 4) << emptyList;

    // Hide columns
    QTest::newRow("first") << emptyList << INT_LIST(0);
    QTest::newRow("middle 1") << emptyList << INT_LIST(1);
    QTest::newRow("middle 3") << emptyList << INT_LIST(3);
    QTest::newRow("last") << emptyList << INT_LIST(4);

    QTest::newRow("subsequent 0,1") << emptyList << INT_LIST(0 << 1);
    QTest::newRow("subsequent 1,2") << emptyList << INT_LIST(1 << 2);
    QTest::newRow("subsequent 3,4") << emptyList << INT_LIST(3 << 4);

    QTest::newRow("all but first") << emptyList << INT_LIST(1 << 2 << 3 << 4);
    QTest::newRow("all but last") << emptyList << INT_LIST(0 << 1 << 2 << 3);
    QTest::newRow("all but middle") << emptyList << INT_LIST(0 << 1 << 3 << 4);

    // Hide both rows and columns at the same time
    QTest::newRow("first") << INT_LIST(0) << INT_LIST(0);
    QTest::newRow("middle 1") << INT_LIST(1) << INT_LIST(1);
    QTest::newRow("middle 3") << INT_LIST(3) << INT_LIST(3);
    QTest::newRow("last") << INT_LIST(4) << INT_LIST(4);

    QTest::newRow("subsequent 0,1") << INT_LIST(0 << 1) << INT_LIST(0 << 1);
    QTest::newRow("subsequent 1,2") << INT_LIST(1 << 2) << INT_LIST(1 << 2);
    QTest::newRow("subsequent 3,4") << INT_LIST(3 << 4) << INT_LIST(3 << 4);

    QTest::newRow("all but first") << INT_LIST(1 << 2 << 3 << 4) << INT_LIST(1 << 2 << 3 << 4);
    QTest::newRow("all but last") << INT_LIST(0 << 1 << 2 << 3) << INT_LIST(0 << 1 << 2 << 3);
    QTest::newRow("all but middle") << INT_LIST(0 << 1 << 3 << 4) << INT_LIST(0 << 1 << 3 << 4);

    // Hide all rows and columns
    QTest::newRow("all") << INT_LIST(0 << 1 << 2 << 3 << 4) << INT_LIST(0 << 1 << 2 << 3 << 4);
}

void tst_QQuickTableView::hideRowsAndColumns()
{
    // Check that you can hide the first row (corner case)
    // and that we load the other columns as expected.
    QFETCH(QVariant, rowsToHide);
    QFETCH(QVariant, columnsToHide);
    LOAD_TABLEVIEW("hiderowsandcolumns.qml");

    const QList<int> rowsToHideList = qvariant_cast<QList<int>>(rowsToHide);
    const QList<int> columnsToHideList = qvariant_cast<QList<int>>(columnsToHide);
    const int modelSize = 5;
    auto model = TestModelAsVariant(modelSize, modelSize);
    view->rootObject()->setProperty("rowsToHide", rowsToHide);
    view->rootObject()->setProperty("columnsToHide", columnsToHide);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    const int expectedRowCount = modelSize - rowsToHideList.count();
    const int expectedColumnCount = modelSize - columnsToHideList.count();
    QCOMPARE(tableViewPrivate->loadedRows.count(), expectedRowCount);
    QCOMPARE(tableViewPrivate->loadedColumns.count(), expectedColumnCount);

    for (const int row : tableViewPrivate->loadedRows.keys())
        QVERIFY(!rowsToHideList.contains(row));

    for (const int column : tableViewPrivate->loadedColumns.keys())
        QVERIFY(!columnsToHideList.contains(column));
}

void tst_QQuickTableView::hideAndShowFirstColumn()
{
    // Check that if we hide the first column, it will move
    // the second column to the origin of the viewport. Then check
    // that if we show the first column again, it will reappear at
    // the origin of the viewport, and as such, pushing the second
    // column to the right of it.
    LOAD_TABLEVIEW("hiderowsandcolumns.qml");

    const int modelSize = 5;
    auto model = TestModelAsVariant(modelSize, modelSize);
    tableView->setModel(model);

    // Start by making the first column hidden
    const auto columnsToHideList = QList<int>() << 0;
    view->rootObject()->setProperty("columnsToHide", QVariant::fromValue(columnsToHideList));

    WAIT_UNTIL_POLISHED;

    const int expectedColumnCount = modelSize - columnsToHideList.count();
    QCOMPARE(tableViewPrivate->loadedColumns.count(), expectedColumnCount);
    QCOMPARE(tableViewPrivate->leftColumn(), 1);
    QCOMPARE(tableView->contentX(), 0);
    QCOMPARE(tableViewPrivate->loadedTableOuterRect.x(), 0);

    // Make the first column in the model visible again
    const auto emptyList = QList<int>();
    view->rootObject()->setProperty("columnsToHide", QVariant::fromValue(emptyList));
    tableView->forceLayout();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->loadedColumns.count(), modelSize);
    QCOMPARE(tableViewPrivate->leftColumn(), 0);
    QCOMPARE(tableView->contentX(), 0);
    QCOMPARE(tableViewPrivate->loadedTableOuterRect.x(), 0);
}

void tst_QQuickTableView::hideAndShowFirstRow()
{
    // Check that if we hide the first row, it will move
    // the second row to the origin of the viewport. Then check
    // that if we show the first row again, it will reappear at
    // the origin of the viewport, and as such, pushing the second
    // row below it.
    LOAD_TABLEVIEW("hiderowsandcolumns.qml");

    const int modelSize = 5;
    auto model = TestModelAsVariant(modelSize, modelSize);
    tableView->setModel(model);

    // Start by making the first row hidden
    const auto rowsToHideList = QList<int>() << 0;
    view->rootObject()->setProperty("rowsToHide", QVariant::fromValue(rowsToHideList));

    WAIT_UNTIL_POLISHED;

    const int expectedRowsCount = modelSize - rowsToHideList.count();
    QCOMPARE(tableViewPrivate->loadedRows.count(), expectedRowsCount);
    QCOMPARE(tableViewPrivate->topRow(), 1);
    QCOMPARE(tableView->contentY(), 0);
    QCOMPARE(tableViewPrivate->loadedTableOuterRect.y(), 0);

    // Make the first row in the model visible again
    const auto emptyList = QList<int>();
    view->rootObject()->setProperty("rowsToHide", QVariant::fromValue(emptyList));
    tableView->forceLayout();

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewPrivate->loadedRows.count(), modelSize);
    QCOMPARE(tableViewPrivate->topRow(), 0);
    QCOMPARE(tableView->contentY(), 0);
    QCOMPARE(tableViewPrivate->loadedTableOuterRect.y(), 0);
}

void tst_QQuickTableView::checkThatRevisionedPropertiesCannotBeUsedInOldImports()
{
    // Check that if you use a QQmlAdaptorModel together with a Repeater, the
    // revisioned context properties 'row' and 'column' are not accessible.
    LOAD_TABLEVIEW("checkmodelpropertyrevision.qml");
    const int resolvedRow = view->rootObject()->property("resolvedDelegateRow").toInt();
    const int resolvedColumn = view->rootObject()->property("resolvedDelegateColumn").toInt();
    QCOMPARE(resolvedRow, 42);
    QCOMPARE(resolvedColumn, 42);
}

void tst_QQuickTableView::checkSyncView_rootView_data()
{
    QTest::addColumn<qreal>("flickToPos");

    QTest::newRow("pos:110") << 110.;
    QTest::newRow("pos:2010") << 2010.;
}

void tst_QQuickTableView::checkSyncView_rootView()
{
    // Check that if you flick on the root tableview (the view that has
    // no other view as syncView), all the other tableviews will sync
    // their content view position according to their syncDirection flag.
    QFETCH(qreal, flickToPos);
    LOAD_TABLEVIEW("syncviewsimple.qml");
    GET_QML_TABLEVIEW(tableViewH);
    GET_QML_TABLEVIEW(tableViewV);
    GET_QML_TABLEVIEW(tableViewHV);
    QQuickTableView *views[] = {tableViewH, tableViewV, tableViewHV};

    auto model = TestModelAsVariant(100, 100);

    tableView->setModel(model);
    for (auto view : views)
        view->setModel(model);

    tableView->setContentX(flickToPos);
    tableView->setContentY(flickToPos);

    WAIT_UNTIL_POLISHED;

    // Check that geometry properties are mirrored
    QCOMPARE(tableViewH->columnSpacing(), tableView->columnSpacing());
    QCOMPARE(tableViewH->rowSpacing(), 0);
    QCOMPARE(tableViewH->contentWidth(), tableView->contentWidth());
    QCOMPARE(tableViewV->columnSpacing(), 0);
    QCOMPARE(tableViewV->rowSpacing(), tableView->rowSpacing());
    QCOMPARE(tableViewV->contentHeight(), tableView->contentHeight());

    // Check that viewport is in sync after the flick
    QCOMPARE(tableView->contentX(), flickToPos);
    QCOMPARE(tableView->contentY(), flickToPos);
    QCOMPARE(tableViewH->contentX(), tableView->contentX());
    QCOMPARE(tableViewH->contentY(), 0);
    QCOMPARE(tableViewV->contentX(), 0);
    QCOMPARE(tableViewV->contentY(), tableView->contentY());
    QCOMPARE(tableViewHV->contentX(), tableView->contentX());
    QCOMPARE(tableViewHV->contentY(), tableView->contentY());

    // Check that topLeft cell is in sync after the flick
    QCOMPARE(tableViewHPrivate->leftColumn(), tableViewPrivate->leftColumn());
    QCOMPARE(tableViewHPrivate->rightColumn(), tableViewPrivate->rightColumn());
    QCOMPARE(tableViewHPrivate->topRow(), 0);
    QCOMPARE(tableViewVPrivate->leftColumn(), 0);
    QCOMPARE(tableViewVPrivate->topRow(), tableViewPrivate->topRow());
    QCOMPARE(tableViewHVPrivate->leftColumn(), tableViewPrivate->leftColumn());
    QCOMPARE(tableViewHVPrivate->topRow(), tableViewPrivate->topRow());

    // Check that the geometry of the tables are in sync after the flick
    QCOMPARE(tableViewHPrivate->loadedTableOuterRect.left(), tableViewPrivate->loadedTableOuterRect.left());
    QCOMPARE(tableViewHPrivate->loadedTableOuterRect.right(), tableViewPrivate->loadedTableOuterRect.right());
    QCOMPARE(tableViewHPrivate->loadedTableOuterRect.top(), 0);

    QCOMPARE(tableViewVPrivate->loadedTableOuterRect.top(), tableViewPrivate->loadedTableOuterRect.top());
    QCOMPARE(tableViewVPrivate->loadedTableOuterRect.bottom(), tableViewPrivate->loadedTableOuterRect.bottom());
    QCOMPARE(tableViewVPrivate->loadedTableOuterRect.left(), 0);

    QCOMPARE(tableViewHVPrivate->loadedTableOuterRect, tableViewPrivate->loadedTableOuterRect);

    // Check that the column widths are in sync
    for (int column = tableView->leftColumn(); column < tableView->rightColumn(); ++column) {
        QCOMPARE(tableViewH->columnWidth(column), tableView->columnWidth(column));
        QCOMPARE(tableViewHV->columnWidth(column), tableView->columnWidth(column));
    }

    // Check that the row heights are in sync
    for (int row = tableView->topRow(); row < tableView->bottomRow(); ++row) {
        QCOMPARE(tableViewV->rowHeight(row), tableView->rowHeight(row));
        QCOMPARE(tableViewHV->rowHeight(row), tableView->rowHeight(row));
    }
}

void tst_QQuickTableView::checkSyncView_childViews_data()
{
    QTest::addColumn<int>("viewIndexToFlick");
    QTest::addColumn<qreal>("flickToPos");

    QTest::newRow("tableViewH, pos:100") << 0 << 100.;
    QTest::newRow("tableViewV, pos:100") << 1 << 100.;
    QTest::newRow("tableViewHV, pos:100") << 2 << 100.;
    QTest::newRow("tableViewH, pos:2000") << 0 << 2000.;
    QTest::newRow("tableViewV, pos:2000") << 1 << 2000.;
    QTest::newRow("tableViewHV, pos:2000") << 2 << 2000.;
}

void tst_QQuickTableView::checkSyncView_childViews()
{
    // Check that if you flick on a tableview that has a syncView, the
    // syncView will move to the new position as well, which will also
    // recursivly move all other connected child views of the syncView.
    QFETCH(int, viewIndexToFlick);
    QFETCH(qreal, flickToPos);
    LOAD_TABLEVIEW("syncviewsimple.qml");
    GET_QML_TABLEVIEW(tableViewH);
    GET_QML_TABLEVIEW(tableViewV);
    GET_QML_TABLEVIEW(tableViewHV);
    QQuickTableView *views[] = {tableViewH, tableViewV, tableViewHV};
    QQuickTableView *viewToFlick = views[viewIndexToFlick];
    QQuickTableViewPrivate *viewToFlickPrivate = QQuickTableViewPrivate::get(viewToFlick);

    auto model = TestModelAsVariant(100, 100);

    tableView->setModel(model);
    for (auto view : views)
        view->setModel(model);

    viewToFlick->setContentX(flickToPos);
    viewToFlick->setContentY(flickToPos);

    WAIT_UNTIL_POLISHED;

    // The view the user flicks on can always be flicked in both directions
    // (unless is has a flickingDirection set, which is not the case here).
    QCOMPARE(viewToFlick->contentX(), flickToPos);
    QCOMPARE(viewToFlick->contentY(), flickToPos);

    // The root view (tableView) will move in sync according
    // to the syncDirection of the view being flicked.
    if (viewToFlick->syncDirection() & Qt::Horizontal) {
        QCOMPARE(tableView->contentX(), flickToPos);
        QCOMPARE(tableViewPrivate->leftColumn(), viewToFlickPrivate->leftColumn());
        QCOMPARE(tableViewPrivate->rightColumn(), viewToFlickPrivate->rightColumn());
        QCOMPARE(tableViewPrivate->loadedTableOuterRect.left(), viewToFlickPrivate->loadedTableOuterRect.left());
        QCOMPARE(tableViewPrivate->loadedTableOuterRect.right(), viewToFlickPrivate->loadedTableOuterRect.right());
    } else {
        QCOMPARE(tableView->contentX(), 0);
        QCOMPARE(tableViewPrivate->leftColumn(), 0);
        QCOMPARE(tableViewPrivate->loadedTableOuterRect.left(), 0);
    }

    if (viewToFlick->syncDirection() & Qt::Vertical) {
        QCOMPARE(tableView->contentY(), flickToPos);
        QCOMPARE(tableViewPrivate->topRow(), viewToFlickPrivate->topRow());
        QCOMPARE(tableViewPrivate->bottomRow(), viewToFlickPrivate->bottomRow());
        QCOMPARE(tableViewPrivate->loadedTableOuterRect.top(), viewToFlickPrivate->loadedTableOuterRect.top());
        QCOMPARE(tableViewPrivate->loadedTableOuterRect.bottom(), viewToFlickPrivate->loadedTableOuterRect.bottom());
    } else {
        QCOMPARE(tableView->contentY(), 0);
        QCOMPARE(tableViewPrivate->topRow(), 0);
        QCOMPARE(tableViewPrivate->loadedTableOuterRect.top(), 0);
    }

    // The other views should continue to stay in sync with
    // the root view, unless it was the view being flicked.
    if (viewToFlick != tableViewH) {
        QCOMPARE(tableViewH->contentX(), tableView->contentX());
        QCOMPARE(tableViewH->contentY(), 0);
        QCOMPARE(tableViewHPrivate->leftColumn(), tableViewPrivate->leftColumn());
        QCOMPARE(tableViewHPrivate->rightColumn(), tableViewPrivate->rightColumn());
        QCOMPARE(tableViewHPrivate->loadedTableOuterRect.left(), tableViewPrivate->loadedTableOuterRect.left());
        QCOMPARE(tableViewHPrivate->loadedTableOuterRect.right(), tableViewPrivate->loadedTableOuterRect.right());
        QCOMPARE(tableViewHPrivate->topRow(), 0);
        QCOMPARE(tableViewHPrivate->loadedTableOuterRect.top(), 0);
    }

    if (viewToFlick != tableViewV) {
        QCOMPARE(tableViewV->contentX(), 0);
        QCOMPARE(tableViewV->contentY(), tableView->contentY());
        QCOMPARE(tableViewVPrivate->topRow(), tableViewPrivate->topRow());
        QCOMPARE(tableViewVPrivate->bottomRow(), tableViewPrivate->bottomRow());
        QCOMPARE(tableViewVPrivate->loadedTableOuterRect.top(), tableViewPrivate->loadedTableOuterRect.top());
        QCOMPARE(tableViewVPrivate->loadedTableOuterRect.bottom(), tableViewPrivate->loadedTableOuterRect.bottom());
        QCOMPARE(tableViewVPrivate->leftColumn(), 0);
        QCOMPARE(tableViewVPrivate->loadedTableOuterRect.left(), 0);
    }

    if (viewToFlick != tableViewHV) {
        QCOMPARE(tableViewHV->contentX(), tableView->contentX());
        QCOMPARE(tableViewHV->contentY(), tableView->contentY());
        QCOMPARE(tableViewHVPrivate->leftColumn(), tableViewPrivate->leftColumn());
        QCOMPARE(tableViewHVPrivate->rightColumn(), tableViewPrivate->rightColumn());
        QCOMPARE(tableViewHVPrivate->topRow(), tableViewPrivate->topRow());
        QCOMPARE(tableViewHVPrivate->bottomRow(), tableViewPrivate->bottomRow());
        QCOMPARE(tableViewHVPrivate->loadedTableOuterRect, tableViewPrivate->loadedTableOuterRect);
    }

    // Check that the column widths are in sync
    for (int column = tableView->leftColumn(); column < tableView->rightColumn(); ++column) {
        QCOMPARE(tableViewH->columnWidth(column), tableView->columnWidth(column));
        QCOMPARE(tableViewHV->columnWidth(column), tableView->columnWidth(column));
    }

    // Check that the row heights are in sync
    for (int row = tableView->topRow(); row < tableView->bottomRow(); ++row) {
        QCOMPARE(tableViewV->rowHeight(row), tableView->rowHeight(row));
        QCOMPARE(tableViewHV->rowHeight(row), tableView->rowHeight(row));
    }
}

void tst_QQuickTableView::checkSyncView_differentSizedModels()
{
    // Check that you can have two tables in a syncView relation, where
    // the sync "child" has fewer rows/columns than the syncView. In that
    // case, it will be possible to flick the syncView further out than
    // the child have rows/columns to follow. This causes some extra
    // challenges for TableView to ensure that they are still kept in
    // sync once you later flick the syncView back to a point where both
    // tables ends up visible. This test will check this sitiation.
    LOAD_TABLEVIEW("syncviewsimple.qml");
    GET_QML_TABLEVIEW(tableViewH);
    GET_QML_TABLEVIEW(tableViewV);
    GET_QML_TABLEVIEW(tableViewHV);

    auto tableViewModel = TestModelAsVariant(100, 100);
    auto tableViewHModel = TestModelAsVariant(100, 50);
    auto tableViewVModel = TestModelAsVariant(50, 100);
    auto tableViewHVModel = TestModelAsVariant(5, 5);

    tableView->setModel(tableViewModel);
    tableViewH->setModel(tableViewHModel);
    tableViewV->setModel(tableViewVModel);
    tableViewHV->setModel(tableViewHVModel);

    WAIT_UNTIL_POLISHED;

    // Flick far out, beyond the smaller tables, which will
    // also force a rebuild (and as such, cause layout properties
    // like average cell width to be temporarily out of sync).
    tableView->setContentX(5000);
    QVERIFY(tableViewPrivate->scheduledRebuildOptions);

    WAIT_UNTIL_POLISHED;

    // Check that the smaller tables are now flicked out of view
    qreal leftEdge = tableViewPrivate->loadedTableOuterRect.left();
    QVERIFY(tableViewHPrivate->loadedTableOuterRect.right() < leftEdge);
    QVERIFY(tableViewHVPrivate->loadedTableOuterRect.right() < leftEdge);

    // Flick slowly back so that we don't trigger a rebuild (since
    // we want to check that we stay in sync also when not rebuilding).
    while (tableView->contentX() > 200) {
        tableView->setContentX(tableView->contentX() - 200);
        QVERIFY(!tableViewPrivate->rebuildOptions);
        QVERIFY(!tableViewPrivate->polishScheduled);
    }

    leftEdge = tableViewPrivate->loadedTableOuterRect.left();
    const int leftColumn = tableViewPrivate->leftColumn();
    QCOMPARE(tableViewHPrivate->loadedTableOuterRect.left(), leftEdge);
    QCOMPARE(tableViewHPrivate->leftColumn(), leftColumn);

    // Because the tableView was fast flicked and then slowly flicked back, the
    // left column is now 49, which is actually far too high, since we're almost
    // at the beginning of the content view. But this "miscalculation" is expected
    // when the column widths are increasing for each column, like they do in this
    // test. In that case, the algorithm that predicts where each column should end
    // up gets slightly confused. Anyway, check that tableViewHV, that has only
    // 5 columns, is not showing any columns, since it should always stay in sync with
    // syncView regardless of the content view position.
    QVERIFY(tableViewHVPrivate->loadedColumns.isEmpty());
}

void tst_QQuickTableView::checkSyncView_differentGeometry()
{
    // Check that you can have two tables in a syncView relation, where
    // the sync "child" is larger than the sync view. This means that the
    // child will display more rows and columns than the parent.
    // In that case, the sync view will anyway need to load the same rows
    // and columns as the child, otherwise the column and row sizes
    // cannot be determined for the child.
    LOAD_TABLEVIEW("syncviewsimple.qml");
    GET_QML_TABLEVIEW(tableViewH);
    GET_QML_TABLEVIEW(tableViewV);
    GET_QML_TABLEVIEW(tableViewHV);

    tableView->setWidth(40);
    tableView->setHeight(40);

    auto tableViewModel = TestModelAsVariant(100, 100);

    tableView->setModel(tableViewModel);
    tableViewH->setModel(tableViewModel);
    tableViewV->setModel(tableViewModel);
    tableViewHV->setModel(tableViewModel);

    WAIT_UNTIL_POLISHED;

    // Check that the column widths are in sync
    for (int column = tableViewH->leftColumn(); column < tableViewH->rightColumn(); ++column) {
        QCOMPARE(tableViewH->columnWidth(column), tableView->columnWidth(column));
        QCOMPARE(tableViewHV->columnWidth(column), tableView->columnWidth(column));
    }

    // Check that the row heights are in sync
    for (int row = tableViewV->topRow(); row < tableViewV->bottomRow(); ++row) {
        QCOMPARE(tableViewV->rowHeight(row), tableView->rowHeight(row));
        QCOMPARE(tableViewHV->rowHeight(row), tableView->rowHeight(row));
    }

    // Flick a bit, and do the same test again
    tableView->setContentX(200);
    tableView->setContentY(200);
    WAIT_UNTIL_POLISHED;

    // Check that the column widths are in sync
    for (int column = tableViewH->leftColumn(); column < tableViewH->rightColumn(); ++column) {
        QCOMPARE(tableViewH->columnWidth(column), tableView->columnWidth(column));
        QCOMPARE(tableViewHV->columnWidth(column), tableView->columnWidth(column));
    }

    // Check that the row heights are in sync
    for (int row = tableViewV->topRow(); row < tableViewV->bottomRow(); ++row) {
        QCOMPARE(tableViewV->rowHeight(row), tableView->rowHeight(row));
        QCOMPARE(tableViewHV->rowHeight(row), tableView->rowHeight(row));
    }
}

void tst_QQuickTableView::checkSyncView_connect_late_data()
{
    QTest::addColumn<qreal>("flickToPos");

    QTest::newRow("pos:110") << 110.;
    QTest::newRow("pos:2010") << 2010.;
}

void tst_QQuickTableView::checkSyncView_connect_late()
{
    // Check that if you assign a syncView to a TableView late, and
    // after the views have been flicked around, they will still end up in sync.
    QFETCH(qreal, flickToPos);
    LOAD_TABLEVIEW("syncviewsimple.qml");
    GET_QML_TABLEVIEW(tableViewH);
    GET_QML_TABLEVIEW(tableViewV);
    GET_QML_TABLEVIEW(tableViewHV);
    QQuickTableView *views[] = {tableViewH, tableViewV, tableViewHV};

    auto model = TestModelAsVariant(100, 100);

    tableView->setModel(model);

    // Start with no syncView connections
    for (auto view : views) {
        view->setSyncView(nullptr);
        view->setModel(model);
    }

    WAIT_UNTIL_POLISHED;

    tableView->setContentX(flickToPos);
    tableView->setContentY(flickToPos);

    WAIT_UNTIL_POLISHED;

    // Check that viewport is not in sync after the flick
    QCOMPARE(tableView->contentX(), flickToPos);
    QCOMPARE(tableView->contentY(), flickToPos);
    QCOMPARE(tableViewH->contentX(), 0);
    QCOMPARE(tableViewH->contentY(), 0);
    QCOMPARE(tableViewV->contentX(), 0);
    QCOMPARE(tableViewV->contentY(), 0);
    QCOMPARE(tableViewHV->contentX(), 0);
    QCOMPARE(tableViewHV->contentY(), 0);

    // Assign the syncView late. This should make
    // all the views end up in sync.
    for (auto view : views) {
        view->setSyncView(tableView);
        WAIT_UNTIL_POLISHED_ARG(view);
    }

    // Check that geometry properties are mirrored
    QCOMPARE(tableViewH->columnSpacing(), tableView->columnSpacing());
    QCOMPARE(tableViewH->rowSpacing(), 0);
    QCOMPARE(tableViewH->contentWidth(), tableView->contentWidth());
    QCOMPARE(tableViewV->columnSpacing(), 0);
    QCOMPARE(tableViewV->rowSpacing(), tableView->rowSpacing());
    QCOMPARE(tableViewV->contentHeight(), tableView->contentHeight());

    // Check that viewport is in sync after the flick
    QCOMPARE(tableView->contentX(), flickToPos);
    QCOMPARE(tableView->contentY(), flickToPos);
    QCOMPARE(tableViewH->contentX(), tableView->contentX());
    QCOMPARE(tableViewH->contentY(), 0);
    QCOMPARE(tableViewV->contentX(), 0);
    QCOMPARE(tableViewV->contentY(), tableView->contentY());
    QCOMPARE(tableViewHV->contentX(), tableView->contentX());
    QCOMPARE(tableViewHV->contentY(), tableView->contentY());

    // Check that topLeft cell is in sync after the flick
    QCOMPARE(tableViewHPrivate->leftColumn(), tableViewPrivate->leftColumn());
    QCOMPARE(tableViewHPrivate->rightColumn(), tableViewPrivate->rightColumn());
    QCOMPARE(tableViewHPrivate->topRow(), 0);
    QCOMPARE(tableViewVPrivate->leftColumn(), 0);
    QCOMPARE(tableViewVPrivate->topRow(), tableViewPrivate->topRow());
    QCOMPARE(tableViewHVPrivate->leftColumn(), tableViewPrivate->leftColumn());
    QCOMPARE(tableViewHVPrivate->topRow(), tableViewPrivate->topRow());

    // Check that the geometry of the tables are in sync after the flick
    QCOMPARE(tableViewHPrivate->loadedTableOuterRect.left(), tableViewPrivate->loadedTableOuterRect.left());
    QCOMPARE(tableViewHPrivate->loadedTableOuterRect.right(), tableViewPrivate->loadedTableOuterRect.right());
    QCOMPARE(tableViewHPrivate->loadedTableOuterRect.top(), 0);

    QCOMPARE(tableViewVPrivate->loadedTableOuterRect.top(), tableViewPrivate->loadedTableOuterRect.top());
    QCOMPARE(tableViewVPrivate->loadedTableOuterRect.bottom(), tableViewPrivate->loadedTableOuterRect.bottom());
    QCOMPARE(tableViewVPrivate->loadedTableOuterRect.left(), 0);

    QCOMPARE(tableViewHVPrivate->loadedTableOuterRect, tableViewPrivate->loadedTableOuterRect);
}

void tst_QQuickTableView::checkSyncView_pageFlicking()
{
    // Check that we rebuild the syncView (instead of refilling
    // edges), if the sync child moves more than a page (the size of TableView).
    // The point is that it shouldn't matter if you fast-flick the
    // sync view itself, or a sync child. Either way, the sync view
    // needs to rebuild. This, in turn, will eventually rebuild the
    // sync children as well when they sync up later.
    LOAD_TABLEVIEW("syncviewsimple.qml");
    GET_QML_TABLEVIEW(tableViewHV);

    auto model = TestModelAsVariant(100, 100);

    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    // Move the viewport more than a "page"
    tableViewHV->setContentX(tableViewHV->width() * 2);

    QVERIFY(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::ViewportOnly);
    QVERIFY(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftColumn);
    QVERIFY(!(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftRow));

    WAIT_UNTIL_POLISHED;

    tableViewHV->setContentY(tableViewHV->height() * 2);

    QVERIFY(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::ViewportOnly);
    QVERIFY(!(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftColumn));
    QVERIFY(tableViewPrivate->scheduledRebuildOptions & QQuickTableViewPrivate::RebuildOption::CalculateNewTopLeftRow);
}

void tst_QQuickTableView::checkSyncView_emptyModel()
{
    // When a tableview has a syncview with an empty model then it should still be
    // showing the tableview without depending on the syncview. This is particularly
    // important for headerviews for example
    LOAD_TABLEVIEW("syncviewsimple.qml");
    GET_QML_TABLEVIEW(tableViewH);
    GET_QML_TABLEVIEW(tableViewV);
    GET_QML_TABLEVIEW(tableViewHV);
    QQuickTableView *views[] = {tableViewH, tableViewV, tableViewHV};

    auto model = TestModelAsVariant(100, 100);

    for (auto view : views)
        view->setModel(model);

    WAIT_UNTIL_POLISHED_ARG(tableViewHV);

    // Check that geometry properties are mirrored
    QCOMPARE(tableViewH->columnSpacing(), tableView->columnSpacing());
    QCOMPARE(tableViewH->rowSpacing(), 0);
    QCOMPARE(tableViewH->contentWidth(), tableView->contentWidth());
    QVERIFY(tableViewH->contentHeight() > 0);
    QCOMPARE(tableViewV->columnSpacing(), 0);
    QCOMPARE(tableViewV->rowSpacing(), tableView->rowSpacing());
    QCOMPARE(tableViewV->contentHeight(), tableView->contentHeight());
    QVERIFY(tableViewV->contentWidth() > 0);

    QCOMPARE(tableViewH->contentX(), tableView->contentX());
    QCOMPARE(tableViewH->contentY(), 0);
    QCOMPARE(tableViewV->contentX(), 0);
    QCOMPARE(tableViewV->contentY(), tableView->contentY());
    QCOMPARE(tableViewHV->contentX(), tableView->contentX());
    QCOMPARE(tableViewHV->contentY(), tableView->contentY());

    QCOMPARE(tableViewHPrivate->loadedTableOuterRect.left(), tableViewPrivate->loadedTableOuterRect.left());
    QCOMPARE(tableViewHPrivate->loadedTableOuterRect.top(), 0);

    QCOMPARE(tableViewVPrivate->loadedTableOuterRect.top(), tableViewPrivate->loadedTableOuterRect.top());
    QCOMPARE(tableViewVPrivate->loadedTableOuterRect.left(), 0);
}

void tst_QQuickTableView::checkSyncView_topLeftChanged()
{
    LOAD_TABLEVIEW("syncviewsimple.qml");
    GET_QML_TABLEVIEW(tableViewH);
    GET_QML_TABLEVIEW(tableViewV);
    GET_QML_TABLEVIEW(tableViewHV);
    QQuickTableView *views[] = {tableViewH, tableViewV, tableViewHV};

    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    for (auto view : views)
        view->setModel(model);

    tableView->setColumnWidthProvider(QJSValue());
    tableView->setRowHeightProvider(QJSValue());
    view->rootObject()->setProperty("delegateWidth", 300);
    view->rootObject()->setProperty("delegateHeight", 300);
    tableView->forceLayout();

    tableViewHV->setContentX(350);
    tableViewHV->setContentY(350);

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableViewH->leftColumn(), tableView->leftColumn());
    QCOMPARE(tableViewV->topRow(), tableView->topRow());

    view->rootObject()->setProperty("delegateWidth", 50);
    view->rootObject()->setProperty("delegateHeight", 50);
    tableView->forceLayout();

    QCOMPARE(tableViewH->leftColumn(), tableView->leftColumn());
    QCOMPARE(tableViewV->topRow(), tableView->topRow());
}

void tst_QQuickTableView::checkThatFetchMoreIsCalledWhenScrolledToTheEndOfTable()
{
    LOAD_TABLEVIEW("plaintableview.qml");

    auto model = TestModelAsVariant(5, 5, true);
    tableView->setModel(model);
    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableView->rows(), 5);
    QCOMPARE(tableView->columns(), 5);

    // Flick table out of view on top
    tableView->setContentX(0);
    tableView->setContentY(-tableView->height() - 10);
    tableView->polish();
    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableView->rows(), 6);
    QCOMPARE(tableView->columns(), 5);
}

void tst_QQuickTableView::delegateWithRequiredProperties()
{
    constexpr static int PositionRole = Qt::UserRole+1;
    struct MyTable : QAbstractTableModel {


        using QAbstractTableModel::QAbstractTableModel;

        int rowCount(const QModelIndex& = QModelIndex()) const override {
            return 3;
        }

        int columnCount(const QModelIndex& = QModelIndex()) const override {
            return 3;
        }

        QVariant data(const QModelIndex &index, int = Qt::DisplayRole) const override {
            return QVariant::fromValue(QString::asprintf("R%d:C%d", index.row(), index.column()));
        }

        QHash<int, QByteArray> roleNames() const override {
            return QHash<int, QByteArray> { {PositionRole, "position"} };
        }
    };

    auto model =  QVariant::fromValue(QSharedPointer<MyTable>(new MyTable));
    {
        QTest::ignoreMessage(QtMsgType::QtInfoMsg, "success");
        LOAD_TABLEVIEW("delegateWithRequired.qml");
        QVERIFY(tableView);
        tableView->setModel(model);
        WAIT_UNTIL_POLISHED;
        QVERIFY(view->errors().empty());
    }
    {
        QTest::ignoreMessage(QtMsgType::QtWarningMsg, QRegularExpression(R"|(TableView: failed loading index: \d)|"));
        LOAD_TABLEVIEW("delegatewithRequiredUnset.qml");
        QVERIFY(tableView);
        tableView->setModel(model);
        WAIT_UNTIL_POLISHED;
        QTRY_VERIFY(view->errors().empty());
    }
}

void tst_QQuickTableView::replaceModel()
{
    LOAD_TABLEVIEW("replaceModelTableView.qml");

    const auto objectModel = view->rootObject()->property("objectModel");
    const auto listModel = view->rootObject()->property("listModel");
    const auto delegateModel = view->rootObject()->property("delegateModel");

    tableView->setModel(listModel);
    QTRY_COMPARE(tableView->rows(), 2);
    tableView->setModel(objectModel);
    QTRY_COMPARE(tableView->rows(), 3);
    tableView->setModel(delegateModel);
    QTRY_COMPARE(tableView->rows(), 2);
    tableView->setModel(listModel);
    QTRY_COMPARE(tableView->rows(), 2);
    tableView->setModel(QVariant());
    QTRY_COMPARE(tableView->rows(), 0);
    QCOMPARE(tableView->contentWidth(), 0);
    QCOMPARE(tableView->contentHeight(), 0);
}

void tst_QQuickTableView::cellAtPos_data()
{
    QTest::addColumn<QPointF>("contentStartPos");
    QTest::addColumn<QPointF>("position");
    QTest::addColumn<bool>("includeSpacing");
    QTest::addColumn<QPoint>("expectedCell");

    const int spacing = 10;
    const QPointF cellSize(100, 50);
    const QPointF halfCell = cellSize / 2;
    const QPointF quadSpace(spacing / 4, spacing / 4);

    auto cellStart = [&](int column, int row){
        const qreal x = (column * (cellSize.x() + spacing));
        const qreal y = (row * (cellSize.y() + spacing));
        return QPointF(x, y);
    };

    QTest::newRow("1") << QPointF(0, 0) << cellStart(0, 0) << false << QPoint(0, 0);
    QTest::newRow("2") << QPointF(0, 0) << cellStart(1, 0) << false << QPoint(1, 0);
    QTest::newRow("3") << QPointF(0, 0) << cellStart(0, 1) << false << QPoint(0, 1);
    QTest::newRow("4") << QPointF(0, 0) << cellStart(1, 1) << false << QPoint(1, 1);

    QTest::newRow("5") << QPointF(0, 0) << cellStart(1, 1) - quadSpace << false << QPoint(-1, -1);
    QTest::newRow("6") << QPointF(0, 0) << cellStart(0, 0) + cellSize + quadSpace << false << QPoint(-1, -1);
    QTest::newRow("7") << QPointF(0, 0) << cellStart(0, 1) + cellSize + quadSpace << false << QPoint(-1, -1);

    QTest::newRow("8") << QPointF(0, 0) << cellStart(1, 1) - quadSpace << true << QPoint(1, 1);
    QTest::newRow("9") << QPointF(0, 0) << cellStart(0, 0) + cellSize + quadSpace << true << QPoint(0, 0);
    QTest::newRow("10") << QPointF(0, 0) << cellStart(0, 1) + cellSize + quadSpace << true << QPoint(0, 1);

    QTest::newRow("11") << cellStart(50, 50) << cellStart(0, 0) << false << QPoint(50, 50);
    QTest::newRow("12") << cellStart(50, 50) << cellStart(4, 4) << false << QPoint(54, 54);
    QTest::newRow("13") << cellStart(50, 50) << cellStart(4, 4) - quadSpace << false << QPoint(-1, -1);
    QTest::newRow("14") << cellStart(50, 50) << cellStart(4, 4) + cellSize + quadSpace << false << QPoint(-1, -1);
    QTest::newRow("15") << cellStart(50, 50) << cellStart(4, 4) - quadSpace << true << QPoint(54, 54);
    QTest::newRow("16") << cellStart(50, 50) << cellStart(4, 4) + cellSize + quadSpace << true << QPoint(54, 54);

    QTest::newRow("17") << cellStart(50, 50) + halfCell << cellStart(0, 0) << false << QPoint(50, 50);
    QTest::newRow("18") << cellStart(50, 50) + halfCell << cellStart(1, 1) << false << QPoint(51, 51);
    QTest::newRow("19") << cellStart(50, 50) + halfCell << cellStart(4, 4) << false << QPoint(54, 54);
}

void tst_QQuickTableView::cellAtPos()
{
    QFETCH(QPointF, contentStartPos);
    QFETCH(QPointF, position);
    QFETCH(bool, includeSpacing);
    QFETCH(QPoint, expectedCell);

    LOAD_TABLEVIEW("plaintableview.qml");
    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);
    tableView->setRowSpacing(10);
    tableView->setColumnSpacing(10);
    tableView->setContentX(contentStartPos.x());
    tableView->setContentY(contentStartPos.y());

    WAIT_UNTIL_POLISHED;

    QPoint cell = tableView->cellAtPos(position, includeSpacing);
    QCOMPARE(cell, expectedCell);
}

void tst_QQuickTableView::positionViewAtRow_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<Qt::AlignmentFlag>("alignment");
    QTest::addColumn<qreal>("offset");
    QTest::addColumn<qreal>("contentYStartPos");

    QTest::newRow("AlignTop 0") << 0 << Qt::AlignTop << 0. << 0.;
    QTest::newRow("AlignTop 1") << 1 << Qt::AlignTop << 0. << 0.;
    QTest::newRow("AlignTop 1") << 1 << Qt::AlignTop << 0. << 50.;
    QTest::newRow("AlignTop 50") << 50 << Qt::AlignTop << 0. << -1.;
    QTest::newRow("AlignTop 0") << 0 << Qt::AlignTop << 0. << -1.;
    QTest::newRow("AlignTop 1") << 1 << Qt::AlignTop << -10. << 0.;
    QTest::newRow("AlignTop 1") << 1 << Qt::AlignTop << -10. << 50.;
    QTest::newRow("AlignTop 50") << 50 << Qt::AlignTop << -10. << -1.;

    QTest::newRow("AlignBottom 50") << 50 << Qt::AlignBottom << 0. << -1.;
    QTest::newRow("AlignBottom 98") << 98 << Qt::AlignBottom << 0. << -1.;
    QTest::newRow("AlignBottom 99") << 99 << Qt::AlignBottom << 0. << -1.;
    QTest::newRow("AlignBottom 50") << 40 << Qt::AlignBottom << 10. << -1.;
    QTest::newRow("AlignBottom 40") << 50 << Qt::AlignBottom << -10. << -1.;
    QTest::newRow("AlignBottom 98") << 98 << Qt::AlignBottom << 10. << -1.;
    QTest::newRow("AlignBottom 99") << 99 << Qt::AlignBottom << -10. << -1.;

    QTest::newRow("AlignCenter 40") << 40 << Qt::AlignCenter << 0. << -1.;
    QTest::newRow("AlignCenter 50") << 50 << Qt::AlignCenter << 0. << -1.;
    QTest::newRow("AlignCenter 40") << 40 << Qt::AlignCenter << 10. << -1.;
    QTest::newRow("AlignCenter 50") << 50 << Qt::AlignCenter << -10. << -1.;
}

void tst_QQuickTableView::positionViewAtRow()
{
    // Check that positionViewAtRow actually flicks the view
    // to the right position so that the row becomes visible.
    // For this test, we only check cells that can be placed exactly
    // according to the given alignment.
    QFETCH(int, row);
    QFETCH(Qt::AlignmentFlag, alignment);
    QFETCH(qreal, offset);
    QFETCH(qreal, contentYStartPos);

    LOAD_TABLEVIEW("plaintableview.qml");
    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);
    if (contentYStartPos >= 0)
        tableView->setContentY(contentYStartPos);

    WAIT_UNTIL_POLISHED;

    tableView->positionViewAtRow(row, alignment, offset);

    WAIT_UNTIL_POLISHED;

    const QPoint cell(0, row);
    const int modelIndex = tableViewPrivate->modelIndexAtCell(cell);
    QVERIFY(tableViewPrivate->loadedItems.contains(modelIndex));
    const auto geometry = tableViewPrivate->loadedTableItem(cell)->geometry();

    switch (alignment) {
    case Qt::AlignTop:
        QCOMPARE(geometry.y(), tableView->contentY() - offset);
        break;
    case Qt::AlignBottom:
        QCOMPARE(geometry.bottom(), tableView->contentY() + tableView->height() - offset);
        break;
    case Qt::AlignCenter:
        QCOMPARE(geometry.y(), tableView->contentY() + (tableView->height() / 2) - (geometry.height() / 2) - offset);
        break;
    default:
        Q_UNREACHABLE();
    }
}

void tst_QQuickTableView::positionViewAtColumn_data()
{
    QTest::addColumn<int>("column");
    QTest::addColumn<Qt::AlignmentFlag>("alignment");
    QTest::addColumn<qreal>("offset");
    QTest::addColumn<qreal>("contentXStartPos");

    QTest::newRow("AlignLeft 0") << 0 << Qt::AlignLeft << 0. << 0.;
    QTest::newRow("AlignLeft 1") << 1 << Qt::AlignLeft << 0. << 0.;
    QTest::newRow("AlignLeft 1") << 1 << Qt::AlignLeft << 0. << 50.;
    QTest::newRow("AlignLeft 50") << 50 << Qt::AlignLeft << 0. << -1.;
    QTest::newRow("AlignLeft 0") << 0 << Qt::AlignLeft << 0. << -1.;
    QTest::newRow("AlignLeft 1") << 1 << Qt::AlignLeft << -10. << 0.;
    QTest::newRow("AlignLeft 1") << 1 << Qt::AlignLeft << -10. << 50.;
    QTest::newRow("AlignLeft 50") << 50 << Qt::AlignLeft << -10. << -1.;

    QTest::newRow("AlignRight 50") << 50 << Qt::AlignRight << 0. << -1.;
    QTest::newRow("AlignRight 99") << 99 << Qt::AlignRight << 0. << -1.;
    QTest::newRow("AlignRight 50") << 50 << Qt::AlignRight << 10. << -1.;
    QTest::newRow("AlignRight 99") << 99 << Qt::AlignRight << -10. << -1.;

    QTest::newRow("AlignCenter 40") << 50 << Qt::AlignCenter << 0. << -1.;
    QTest::newRow("AlignCenter 50") << 50 << Qt::AlignCenter << 0. << -1.;
    QTest::newRow("AlignCenter 40") << 50 << Qt::AlignCenter << 10. << -1.;
    QTest::newRow("AlignCenter 50") << 50 << Qt::AlignCenter << -10. << -1.;
}

void tst_QQuickTableView::positionViewAtColumn()
{
    // Check that positionViewAtColumn actually flicks the view
    // to the right position so that the row becomes visible.
    // For this test, we only check cells that can be placed exactly
    // according to the given alignment.
    QFETCH(int, column);
    QFETCH(Qt::AlignmentFlag, alignment);
    QFETCH(qreal, offset);
    QFETCH(qreal, contentXStartPos);

    LOAD_TABLEVIEW("plaintableview.qml");
    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);
    if (contentXStartPos >= 0)
        tableView->setContentX(contentXStartPos);

    WAIT_UNTIL_POLISHED;

    tableView->positionViewAtColumn(column, alignment, offset);

    WAIT_UNTIL_POLISHED;

    const QPoint cell(column, 0);
    const int modelIndex = tableViewPrivate->modelIndexAtCell(cell);
    QVERIFY(tableViewPrivate->loadedItems.contains(modelIndex));
    const auto geometry = tableViewPrivate->loadedTableItem(cell)->geometry();

    switch (alignment) {
    case Qt::AlignLeft:
        QCOMPARE(geometry.x(), tableView->contentX() - offset);
        break;
    case Qt::AlignRight:
        QCOMPARE(geometry.right(), tableView->contentX() + tableView->width() - offset);
        break;
    case Qt::AlignCenter:
        QCOMPARE(geometry.x(), tableView->contentX() + (tableView->width() / 2) - (geometry.width() / 2) - offset);
        break;
    default:
        Q_UNREACHABLE();
    }
}

void tst_QQuickTableView::positionViewAtRowClamped_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<Qt::AlignmentFlag>("alignment");
    QTest::addColumn<qreal>("offset");
    QTest::addColumn<qreal>("contentYStartPos");

    QTest::newRow("AlignTop 0") << 0 << Qt::AlignTop << -10. << 0.;
    QTest::newRow("AlignTop 0") << 0 << Qt::AlignTop << -10. << -1.;
    QTest::newRow("AlignTop 99") << 99 << Qt::AlignTop << 0. << -1.;
    QTest::newRow("AlignTop 99") << 99 << Qt::AlignTop << -10. << -1.;

    QTest::newRow("AlignBottom 0") << 0 << Qt::AlignBottom << 0. << 0.;
    QTest::newRow("AlignBottom 1") << 1 << Qt::AlignBottom << 0. << 0.;
    QTest::newRow("AlignBottom 1") << 1 << Qt::AlignBottom << 0. << 50.;
    QTest::newRow("AlignBottom 0") << 0 << Qt::AlignBottom << 0. << -1.;

    QTest::newRow("AlignBottom 0") << 0 << Qt::AlignBottom << 10. << 0.;
    QTest::newRow("AlignBottom 1") << 1 << Qt::AlignBottom << 10. << 0.;
    QTest::newRow("AlignBottom 1") << 1 << Qt::AlignBottom << 10. << 50.;
    QTest::newRow("AlignBottom 0") << 0 << Qt::AlignBottom << 10. << -1.;
    QTest::newRow("AlignBottom 99") << 99 << Qt::AlignBottom << 10. << -1.;

    QTest::newRow("AlignCenter 0") << 0 << Qt::AlignCenter << 0. << 0.;
    QTest::newRow("AlignCenter 1") << 1 << Qt::AlignCenter << 0. << 0.;
    QTest::newRow("AlignCenter 1") << 1 << Qt::AlignCenter << 0. << 50.;
    QTest::newRow("AlignCenter 0") << 0 << Qt::AlignCenter << 0. << -1.;
    QTest::newRow("AlignCenter 99") << 99 << Qt::AlignCenter << 0. << -1.;

    QTest::newRow("AlignCenter 0") << 0 << Qt::AlignCenter << -10. << 0.;
    QTest::newRow("AlignCenter 1") << 1 << Qt::AlignCenter << -10. << 0.;
    QTest::newRow("AlignCenter 1") << 1 << Qt::AlignCenter << -10. << 50.;
    QTest::newRow("AlignCenter 0") << 0 << Qt::AlignCenter << -10. << -1.;
    QTest::newRow("AlignCenter 99") << 99 << Qt::AlignCenter << -10. << -1.;
}

void tst_QQuickTableView::positionViewAtRowClamped()
{
    // Check that positionViewAtRow actually flicks the table to the
    // right position so that the row becomes visible. For this test, we
    // only test cells that cannot be placed exactly at the given alignment,
    // because it would cause the table to overshoot. Instead the
    // table should be flicked to the edge of the viewport, close to the
    // requested alignment.
    QFETCH(int, row);
    QFETCH(Qt::AlignmentFlag, alignment);
    QFETCH(qreal, offset);
    QFETCH(qreal, contentYStartPos);

    LOAD_TABLEVIEW("plaintableview.qml");
    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);
    if (contentYStartPos >= 0)
        tableView->setContentY(contentYStartPos);

    WAIT_UNTIL_POLISHED;

    tableView->positionViewAtRow(row, alignment, offset);

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableView->contentY(), row < 50 ? 0 : tableView->contentHeight() - tableView->height());
}

void tst_QQuickTableView::positionViewAtColumnClamped_data()
{
    QTest::addColumn<int>("column");
    QTest::addColumn<Qt::AlignmentFlag>("alignment");
    QTest::addColumn<qreal>("offset");
    QTest::addColumn<qreal>("contentXStartPos");

    QTest::newRow("AlignLeft 0") << 0 << Qt::AlignLeft << -10. << 0.;
    QTest::newRow("AlignLeft 0") << 0 << Qt::AlignLeft << -10. << -1.;
    QTest::newRow("AlignLeft 99") << 99 << Qt::AlignLeft << 0. << -1.;
    QTest::newRow("AlignLeft 99") << 99 << Qt::AlignLeft << -10. << -1.;

    QTest::newRow("AlignRight 0") << 0 << Qt::AlignRight << 0. << 0.;
    QTest::newRow("AlignRight 1") << 1 << Qt::AlignRight << 0. << 0.;
    QTest::newRow("AlignRight 1") << 1 << Qt::AlignRight << 0. << 50.;
    QTest::newRow("AlignRight 0") << 0 << Qt::AlignRight << 0. << -1.;

    QTest::newRow("AlignRight 0") << 0 << Qt::AlignRight << 10. << 0.;
    QTest::newRow("AlignRight 1") << 1 << Qt::AlignRight << 10. << 0.;
    QTest::newRow("AlignRight 1") << 1 << Qt::AlignRight << 10. << 50.;
    QTest::newRow("AlignRight 0") << 0 << Qt::AlignRight << 10. << -1.;
    QTest::newRow("AlignRight 99") << 99 << Qt::AlignRight << 10. << -1.;

    QTest::newRow("AlignCenter 0") << 0 << Qt::AlignCenter << 0. << 0.;
    QTest::newRow("AlignCenter 1") << 1 << Qt::AlignCenter << 0. << 0.;
    QTest::newRow("AlignCenter 1") << 1 << Qt::AlignCenter << 0. << 50.;
    QTest::newRow("AlignCenter 0") << 0 << Qt::AlignCenter << 0. << -1.;
    QTest::newRow("AlignCenter 99") << 99 << Qt::AlignCenter << 0. << -1.;

    QTest::newRow("AlignCenter 0") << 0 << Qt::AlignCenter << -10. << 0.;
    QTest::newRow("AlignCenter 1") << 1 << Qt::AlignCenter << -10. << 0.;
    QTest::newRow("AlignCenter 1") << 1 << Qt::AlignCenter << -10. << 50.;
    QTest::newRow("AlignCenter 0") << 0 << Qt::AlignCenter << -10. << -1.;
    QTest::newRow("AlignCenter 99") << 99 << Qt::AlignCenter << -10. << -1.;
}

void tst_QQuickTableView::positionViewAtColumnClamped()
{
    // Check that positionViewAtColumn actually flicks the table to the
    // right position so that the column becomes visible. For this test, we
    // only test cells that cannot be placed exactly at the given alignment,
    // because it would cause the table to overshoot. Instead the
    // table should be flicked to the edge of the viewport, close to the
    // requested alignment.
    QFETCH(int, column);
    QFETCH(Qt::AlignmentFlag, alignment);
    QFETCH(qreal, offset);
    QFETCH(qreal, contentXStartPos);

    LOAD_TABLEVIEW("plaintableview.qml");
    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);
    if (contentXStartPos >= 0)
        tableView->setContentX(contentXStartPos);

    WAIT_UNTIL_POLISHED;

    tableView->positionViewAtColumn(column, alignment, offset);

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableView->contentX(), column < 50 ? 0 : tableView->contentWidth() - tableView->width());
}

void tst_QQuickTableView::positionViewAtLastRow_data()
{
    QTest::addColumn<QString>("signalToTest");

    QTest::newRow("positionOnRowsChanged") << "positionOnRowsChanged";
    QTest::newRow("positionOnContentHeightChanged") << "positionOnContentHeightChanged";
}

void tst_QQuickTableView::positionViewAtLastRow()
{
    // Check that we can make TableView always scroll to the
    // last row in the model by positioning the view upon
    // a rowsChanged callback
    QFETCH(QString, signalToTest);

    LOAD_TABLEVIEW("positionlast.qml");

    // Use a very large model to indirectly test that we "fast-flick" to
    // the end at start-up (instead of loading and unloading rows, which
    // would take forever).
    TestModel model(2000000, 2000000);
    tableView->setModel(QVariant::fromValue(&model));

    view->rootObject()->setProperty(signalToTest.toUtf8().constData(), true);

    WAIT_UNTIL_POLISHED;

    const qreal delegateSize = 100.;
    const qreal viewportRowCount = tableView->height() / delegateSize;

    // Check that the viewport is positioned at the last row at start-up
    QCOMPARE(tableView->rows(), model.rowCount());
    QCOMPARE(tableView->bottomRow(), model.rowCount() - 1);
    QCOMPARE(tableView->contentY(), (model.rowCount() - viewportRowCount) * delegateSize);

    // Check that the viewport is positioned at the last
    // row after more rows are added.
    for (int row = 0; row < 2; ++row) {
        model.addRow(model.rowCount() - 1);

        WAIT_UNTIL_POLISHED;

        QCOMPARE(tableView->rows(), model.rowCount());
        QCOMPARE(tableView->bottomRow(), model.rowCount() - 1);
        QCOMPARE(tableView->contentY(), (model.rowCount() - viewportRowCount) * delegateSize);
    }
}

void tst_QQuickTableView::positionViewAtLastColumn_data()
{
    QTest::addColumn<QString>("signalToTest");

    QTest::newRow("positionOnColumnsChanged") << "positionOnColumnsChanged";
    QTest::newRow("positionOnContentWidthChanged") << "positionOnContentWidthChanged";
}

void tst_QQuickTableView::positionViewAtLastColumn()
{
    // Check that we can make TableView always scroll to the
    // last column in the model by positioning the view upon
    // a columnsChanged callback
    QFETCH(QString, signalToTest);

    LOAD_TABLEVIEW("positionlast.qml");

    // Use a very large model to indirectly test that we "fast-flick" to
    // the end at start-up (instead of loading and unloading columns, which
    // would take forever).
    TestModel model(2000000, 2000000);
    tableView->setModel(QVariant::fromValue(&model));

    view->rootObject()->setProperty(signalToTest.toUtf8().constData(), true);

    WAIT_UNTIL_POLISHED;

    const qreal delegateSize = 100.;
    const qreal viewportColumnCount = tableView->width() / delegateSize;

    // Check that the viewport is positioned at the last column at start-up
    QCOMPARE(tableView->columns(), model.columnCount());
    QCOMPARE(tableView->rightColumn(), model.columnCount() - 1);
    QCOMPARE(tableView->contentX(), (model.columnCount() - viewportColumnCount) * delegateSize);

    // Check that the viewport is positioned at the last
    // column after more columns are added.
    for (int column = 0; column < 2; ++column) {
        model.addColumn(model.columnCount() - 1);

        WAIT_UNTIL_POLISHED;

        QCOMPARE(tableView->columns(), model.columnCount());
        QCOMPARE(tableView->rightColumn(), model.columnCount() - 1);
        QCOMPARE(tableView->contentX(), (model.columnCount() - viewportColumnCount) * delegateSize);
    }
}

void tst_QQuickTableView::itemAtCell_data()
{
    QTest::addColumn<QPoint>("cell");
    QTest::addColumn<bool>("shouldExist");

    QTest::newRow("0, 0") << QPoint(0, 0) << true;
    QTest::newRow("0, 4") << QPoint(0, 4) << true;
    QTest::newRow("4, 0") << QPoint(4, 0) << true;
    QTest::newRow("4, 4") << QPoint(4, 4) << true;
    QTest::newRow("30, 30") << QPoint(30, 30) << false;
    QTest::newRow("-1, -1") << QPoint(-1, -1) << false;
}

void tst_QQuickTableView::itemAtCell()
{
    QFETCH(QPoint, cell);
    QFETCH(bool, shouldExist);

    LOAD_TABLEVIEW("plaintableview.qml");
    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    WAIT_UNTIL_POLISHED;

    const auto item = tableView->itemAtCell(cell);
    if (shouldExist) {
        const auto context = qmlContext(item);
        const int contextRow = context->contextProperty("row").toInt();
        const int contextColumn = context->contextProperty("column").toInt();
        QCOMPARE(contextColumn, cell.x());
        QCOMPARE(contextRow, cell.y());
    } else {
        QVERIFY(!item);
    }
}

void tst_QQuickTableView::leftRightTopBottomProperties_data()
{
    QTest::addColumn<QPointF>("contentStartPos");
    QTest::addColumn<QMargins>("expectedTable");
    QTest::addColumn<QMargins>("expectedSignalCount");

    QTest::newRow("1") << QPointF(0, 0) << QMargins(0, 0, 5, 7) << QMargins(0, 0, 1, 1);
    QTest::newRow("2") << QPointF(100, 50) << QMargins(1, 1, 6, 8) << QMargins(1, 1, 2, 2);
    QTest::newRow("3") << QPointF(220, 120) << QMargins(2, 2, 8, 10) << QMargins(2, 2, 4, 4);
    QTest::newRow("4") << QPointF(1000, 1000) << QMargins(9, 19, 15, 27) << QMargins(1, 1, 2, 2);
}

void tst_QQuickTableView::leftRightTopBottomProperties()
{
    QFETCH(QPointF, contentStartPos);
    QFETCH(QMargins, expectedTable);
    QFETCH(QMargins, expectedSignalCount);

    LOAD_TABLEVIEW("plaintableview.qml");
    auto model = TestModelAsVariant(100, 100);
    tableView->setModel(model);

    QSignalSpy leftSpy(tableView, &QQuickTableView::leftColumnChanged);
    QSignalSpy rightSpy(tableView, &QQuickTableView::rightColumnChanged);
    QSignalSpy topSpy(tableView, &QQuickTableView::topRowChanged);
    QSignalSpy bottomSpy(tableView, &QQuickTableView::bottomRowChanged);

    WAIT_UNTIL_POLISHED;

    tableView->setContentX(contentStartPos.x());
    tableView->setContentY(contentStartPos.y());

    tableView->polish();
    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableView->leftColumn(), expectedTable.left());
    QCOMPARE(tableView->topRow(), expectedTable.top());
    QCOMPARE(tableView->rightColumn(), expectedTable.right());
    QCOMPARE(tableView->bottomRow(), expectedTable.bottom());

    QCOMPARE(leftSpy.count(), expectedSignalCount.left());
    QCOMPARE(rightSpy.count(), expectedSignalCount.right());
    QCOMPARE(topSpy.count(), expectedSignalCount.top());
    QCOMPARE(bottomSpy.count(), expectedSignalCount.bottom());
}

void tst_QQuickTableView::checkContentSize_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("colCount");

    QTest::newRow("4x4") << 4 << 4;
    QTest::newRow("100x100") << 100 << 100;
    QTest::newRow("0x0") << 0 << 0;
}

void tst_QQuickTableView::checkContentSize()
{
    QFETCH(int, rowCount);
    QFETCH(int, colCount);

    // Check that the content size is initially correct, and that
    // it updates when we change e.g the model or spacing (QTBUG-87680)
    LOAD_TABLEVIEW("plaintableview.qml");

    TestModel model(rowCount, colCount);
    tableView->setModel(QVariant::fromValue(&model));
    tableView->setRowSpacing(1);
    tableView->setColumnSpacing(2);

    WAIT_UNTIL_POLISHED;

    const qreal delegateWidth = 100;
    const qreal delegateHeight = 50;
    qreal colSpacing = tableView->columnSpacing();
    qreal rowSpacing = tableView->rowSpacing();

    // Check that content size has the exepected initial values
    QCOMPARE(tableView->contentWidth(), colCount == 0 ? 0 : (colCount * (delegateWidth + colSpacing)) - colSpacing);
    QCOMPARE(tableView->contentHeight(), rowCount == 0 ? 0 : (rowCount * (delegateHeight + rowSpacing)) - rowSpacing);

    // Set no spacing
    rowSpacing = 0;
    colSpacing = 0;
    tableView->setRowSpacing(rowSpacing);
    tableView->setColumnSpacing(colSpacing);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->contentWidth(), colCount * delegateWidth);
    QCOMPARE(tableView->contentHeight(), rowCount * delegateHeight);

    // Set typical spacing values
    rowSpacing = 2;
    colSpacing = 3;
    tableView->setRowSpacing(rowSpacing);
    tableView->setColumnSpacing(colSpacing);
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->contentWidth(), colCount == 0 ? 0 : (colCount * (delegateWidth + colSpacing)) - colSpacing);
    QCOMPARE(tableView->contentHeight(), rowCount == 0 ? 0 : (rowCount * (delegateHeight + rowSpacing)) - rowSpacing);

    // Add a row and a column
    model.insertRow(0);
    model.insertColumn(0);
    rowCount = model.rowCount();
    colCount = model.columnCount();
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->contentWidth(), (colCount * (delegateWidth + colSpacing)) - colSpacing);
    QCOMPARE(tableView->contentHeight(), (rowCount * (delegateHeight + rowSpacing)) - rowSpacing);

    // Remove a row (this should also make affect contentWidth if rowCount becomes 0)
    model.removeRow(0);
    rowCount = model.rowCount();
    WAIT_UNTIL_POLISHED;
    QCOMPARE(tableView->contentWidth(), rowCount == 0 ? 0 : (colCount * (delegateWidth + colSpacing)) - colSpacing);
    QCOMPARE(tableView->contentHeight(), rowCount == 0 ? 0 : (rowCount * (delegateHeight + rowSpacing)) - rowSpacing);
}

void tst_QQuickTableView::checkSelectionModelWithRequiredSelectedProperty_data()
{
    QTest::addColumn<QVector<QPoint>>("selected");
    QTest::addColumn<QPoint>("toggle");

    QTest::newRow("nothing selected") << QVector<QPoint>() << QPoint(0,0);
    QTest::newRow("one item selected") << (QVector<QPoint>() << QPoint(0, 0)) << QPoint(1, 1);
    QTest::newRow("two items selected") << (QVector<QPoint>() << QPoint(1, 1) << QPoint(2, 2)) << QPoint(1, 1);
}

void tst_QQuickTableView::checkSelectionModelWithRequiredSelectedProperty()
{
    // Check that if you add a "required property selected" to the delegate,
    // TableView will give it a value upon creation that matches the state
    // in the selection model.
    QFETCH(QVector<QPoint>, selected);
    QFETCH(QPoint, toggle);

    LOAD_TABLEVIEW("tableviewwithselected1.qml");

    TestModel model(10, 10);
    QItemSelectionModel selectionModel(&model);

    // Set initially selected cells
    for (auto it = selected.constBegin(); it != selected.constEnd(); ++it) {
        const QPoint &cell = *it;
        selectionModel.select(model.index(cell.y(), cell.x()), QItemSelectionModel::Select);
    }

    tableView->setModel(QVariant::fromValue(&model));
    tableView->setSelectionModel(&selectionModel);

    WAIT_UNTIL_POLISHED;

    // Check that all delegates have "selected" set with the initial value
    for (auto fxItem : tableViewPrivate->loadedItems) {
        const auto context = qmlContext(fxItem->item.data());
        const int row = context->contextProperty("row").toInt();
        const int column = context->contextProperty("column").toInt();
        const bool selected = fxItem->item->property("selected").toBool();
        const auto modelIndex = model.index(row, column);
        QCOMPARE(selected, selectionModel.isSelected(modelIndex));
    }

    // Toggle selected on one of the model indices, and check
    // that the "selected" property got updated as well
    const QModelIndex toggleIndex = model.index(toggle.y(), toggle.x());
    const bool wasSelected = selectionModel.isSelected(toggleIndex);
    selectionModel.select(toggleIndex, QItemSelectionModel::Toggle);
    const auto fxItem = tableViewPrivate->loadedTableItem(toggle);
    const bool isSelected = fxItem->item->property("selected").toBool();
    QCOMPARE(isSelected, !wasSelected);
}

void tst_QQuickTableView::checkSelectionModelWithUnrequiredSelectedProperty()
{
    // Check that if there is a property "selected" in the delegate, but it's
    // not required, then TableView will not touch it. This is for legacy reasons, to
    // not break applications written before Qt 6.2 that has such a property
    // added for application logic.
    LOAD_TABLEVIEW("tableviewwithselected2.qml");

    TestModel model(10, 10);
    tableView->setModel(QVariant::fromValue(&model));
    QItemSelectionModel *selectionModel = tableView->selectionModel();
    QVERIFY(selectionModel);

    // Select a cell
    selectionModel->select(model.index(1, 1), QItemSelectionModel::Select);

    WAIT_UNTIL_POLISHED;

    const auto fxItem = tableViewPrivate->loadedTableItem(QPoint(1, 1));
    const bool selected = fxItem->item->property("selected").toBool();
    QCOMPARE(selected, false);
}

void tst_QQuickTableView::removeAndAddSelectionModel()
{
    // Check that if we remove the selection model from TableView, all delegates
    // will be unselected. And opposite, if we add the selection model back, the
    // delegates will be updated.
    LOAD_TABLEVIEW("tableviewwithselected1.qml");

    TestModel model(10, 10);
    QItemSelectionModel selectionModel(&model);

    // Select a cell in the selection model
    selectionModel.select(model.index(1, 1), QItemSelectionModel::Select);

    tableView->setModel(QVariant::fromValue(&model));
    tableView->setSelectionModel(&selectionModel);

    WAIT_UNTIL_POLISHED;

    // Check that the delegate item is selected
    const auto fxItem = tableViewPrivate->loadedTableItem(QPoint(1, 1));
    bool selected = fxItem->item->property("selected").toBool();
    QCOMPARE(selected, true);

    // Remove the selection model, and check that the delegate item is now unselected
    tableView->setSelectionModel(nullptr);
    selected = fxItem->item->property("selected").toBool();
    QCOMPARE(selected, false);

    // Add the selection model back, and check that the delegate item is selected again
    tableView->setSelectionModel(&selectionModel);
    selected = fxItem->item->property("selected").toBool();
    QCOMPARE(selected, true);
}

void tst_QQuickTableView::testSelectableStartPosEndPos_data()
{
    QTest::addColumn<QPoint>("endCellDist");

    QTest::newRow("single cell") << QPoint(0, 0);

    QTest::newRow("left to right") << QPoint(1, 0);
    QTest::newRow("left to right") << QPoint(2, 0);
    QTest::newRow("right to left") << QPoint(-1, 0);
    QTest::newRow("right to left") << QPoint(-2, 0);

    QTest::newRow("top to bottom") << QPoint(0, 1);
    QTest::newRow("top to bottom") << QPoint(0, 2);
    QTest::newRow("bottom to top") << QPoint(0, -1);
    QTest::newRow("bottom to top") << QPoint(0, -2);

    QTest::newRow("diagonal top left to bottom right") << QPoint(1, 1);
    QTest::newRow("diagonal top left to bottom right") << QPoint(2, 2);
    QTest::newRow("diagonal bottom left to top right") << QPoint(-1, -1);
    QTest::newRow("diagonal bottom left to top right") << QPoint(-2, -2);
    QTest::newRow("diagonal top right to bottom left") << QPoint(-1, 1);
    QTest::newRow("diagonal top right to bottom left") << QPoint(-2, 2);
    QTest::newRow("diagonal bottom right to top left") << QPoint(1, -1);
    QTest::newRow("diagonal bottom right to top left") << QPoint(2, -2);
}

void tst_QQuickTableView::testSelectableStartPosEndPos()
{
    // Check that the TableView implement QQuickSelectableInterface setSelectionStartPos, setSelectionEndPos
    // and clearSelection correctly. Do this by calling setSelectionStartPos/setSelectionEndPos on top of
    // different cells, and see that we end up with the expected selections.
    QFETCH(QPoint, endCellDist);
    LOAD_TABLEVIEW("tableviewwithselected1.qml");

    TestModel model(10, 10);
    QItemSelectionModel selectionModel(&model);

    tableView->setModel(QVariant::fromValue(&model));
    tableView->setSelectionModel(&selectionModel);

    WAIT_UNTIL_POLISHED;

    QCOMPARE(selectionModel.hasSelection(), false);

    const QPoint startCell(5, 5);
    const QPoint endCell = startCell + endCellDist;
    const QPoint endCellWrapped = startCell - endCellDist;

    const QQuickItem *startItem = tableView->itemAtCell(startCell);
    const QQuickItem *endItem = tableView->itemAtCell(endCell);
    const QQuickItem *endItemWrapped = tableView->itemAtCell(endCellWrapped);
    QVERIFY(startItem);
    QVERIFY(endItem);
    QVERIFY(endItemWrapped);

    const QPointF startPos(startItem->x(), startItem->y());
    const QPointF endPos(endItem->x(), endItem->y());
    const QPointF endPosWrapped(endItemWrapped->x(), endItemWrapped->y());

    tableViewPrivate->setSelectionStartPos(startPos);
    tableViewPrivate->setSelectionEndPos(endPos);

    QCOMPARE(selectionModel.hasSelection(), true);

    const int x1 = qMin(startCell.x(), endCell.x());
    const int x2 = qMax(startCell.x(), endCell.x());
    const int y1 = qMin(startCell.y(), endCell.y());
    const int y2 = qMax(startCell.y(), endCell.y());

    for (int x = x1; x < x2; ++x) {
        for (int y = y1; y < y2; ++y) {
            const auto index = model.index(y, x);
            QVERIFY(selectionModel.isSelected(index));
        }
    }

    const int expectedCount = (x2 - x1 + 1) * (y2 - y1 + 1);
    const int actualCount = selectionModel.selectedIndexes().count();
    QCOMPARE(actualCount, expectedCount);

    // Wrap the selection
    tableViewPrivate->setSelectionEndPos(endPosWrapped);

    for (int x = x2; x < x1; ++x) {
        for (int y = y2; y < y1; ++y) {
            const auto index = model.index(y, x);
            QVERIFY(selectionModel.isSelected(index));
        }
    }

    const int actualCountAfterWrap = selectionModel.selectedIndexes().count();
    QCOMPARE(actualCountAfterWrap, expectedCount);

    tableViewPrivate->clearSelection();
    QCOMPARE(selectionModel.hasSelection(), false);
}

void tst_QQuickTableView::testSelectableStartPosEndPosOutsideView()
{
    // Call setSelectionStartPos and setSelectionEndPos with positions outside the view.
    // This should first of all not crash, but instead just clamp the selection to the
    // cells that are visible inside the view.
    LOAD_TABLEVIEW("tableviewwithselected1.qml");

    TestModel model(10, 10);
    QItemSelectionModel selectionModel(&model);

    tableView->setModel(QVariant::fromValue(&model));
    tableView->setSelectionModel(&selectionModel);

    WAIT_UNTIL_POLISHED;

    const QPoint centerCell(5, 5);
    const QQuickItem *centerItem = tableView->itemAtCell(centerCell);
    QVERIFY(centerItem);

    const QPointF centerPos(centerItem->x(), centerItem->y());
    const QPointF outsideLeft(-100, centerPos.y());
    const QPointF outsideRight(tableView->width() + 100, centerPos.y());
    const QPointF outsideTop(centerPos.x(), -100);
    const QPointF outsideBottom(centerPos.x(), tableView->height() + 100);

    tableViewPrivate->setSelectionStartPos(centerPos);

    tableViewPrivate->setSelectionEndPos(outsideLeft);
    for (int x = 0; x <= centerCell.x(); ++x) {
        const auto index = model.index(centerCell.y(), x);
        QVERIFY(selectionModel.isSelected(index));
    }

    tableViewPrivate->setSelectionEndPos(outsideRight);
    for (int x = centerCell.x(); x < model.columnCount(); ++x) {
        const auto index = model.index(centerCell.y(), x);
        QVERIFY(selectionModel.isSelected(index));
    }

    tableViewPrivate->setSelectionEndPos(outsideTop);
    for (int y = 0; y <= centerCell.y(); ++y) {
        const auto index = model.index(y, centerCell.x());
        QVERIFY(selectionModel.isSelected(index));
    }

    tableViewPrivate->setSelectionEndPos(outsideBottom);
    for (int y = centerCell.y(); y < model.rowCount(); ++y) {
        const auto index = model.index(y, centerCell.x());
        QVERIFY(selectionModel.isSelected(index));
    }
}

void tst_QQuickTableView::testSelectableScrollTowardsPos()
{
    // Check that TableView will implement the scrollTowardsSelectionPoint function
    // correctly, and move the content item towards the given position
    LOAD_TABLEVIEW("tableviewwithselected1.qml");

    TestModel model(200, 200);
    QItemSelectionModel selectionModel(&model);

    tableView->setModel(QVariant::fromValue(&model));
    tableView->setSelectionModel(&selectionModel);

    WAIT_UNTIL_POLISHED;

    QCOMPARE(tableView->contentX(), 0);
    QCOMPARE(tableView->contentY(), 0);

    const QSizeF step(1, 1);
    const QPointF topLeft(-100, -100);
    const QPointF topRight(tableView->width() + 100, -100);
    const QPointF bottomLeft(-100, tableView->height() + 100);
    const QPointF bottomRight(tableView->width() + 100, tableView->height() + 100);

    tableViewPrivate->scrollTowardsSelectionPoint(topRight, step);
    QCOMPARE(tableView->contentX(), step.width());
    QCOMPARE(tableView->contentY(), 0);

    tableViewPrivate->scrollTowardsSelectionPoint(bottomRight, step);
    QCOMPARE(tableView->contentX(), step.width() * 2);
    QCOMPARE(tableView->contentY(), step.height());

    tableViewPrivate->scrollTowardsSelectionPoint(bottomLeft, step);
    QCOMPARE(tableView->contentX(), step.width());
    QCOMPARE(tableView->contentY(), step.height() * 2);

    tableViewPrivate->scrollTowardsSelectionPoint(topLeft, step);
    QCOMPARE(tableView->contentX(), 0);
    QCOMPARE(tableView->contentY(), step.height());

    tableViewPrivate->scrollTowardsSelectionPoint(topLeft, step);
    QCOMPARE(tableView->contentX(), 0);
    QCOMPARE(tableView->contentY(), 0);
}

void tst_QQuickTableView::resettingRolesRespected()
{
    LOAD_TABLEVIEW("resetModelData.qml");

    TestModel model(1, 1);
    tableView->setModel(QVariant::fromValue(&model));

    WAIT_UNTIL_POLISHED;

    QVERIFY(!tableView->property("success").toBool());
    model.useCustomRoleNames(true);
    QTRY_VERIFY(tableView->property("success").toBool());
}

void tst_QQuickTableView::deletedDelegate()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("deletedDelegate.qml"));
    std::unique_ptr<QObject> root(component.create());
    QVERIFY(root);
    auto tv = root->findChild<QQuickTableView *>("tableview");
    QVERIFY(tv);
    // we need one event loop iteration for the deferred delete to trigger
    // thus the QTRY_VERIFY
    QTRY_COMPARE(tv->delegate(), nullptr);
}

void tst_QQuickTableView::checkRebuildJsModel()
{
    LOAD_TABLEVIEW("resetJsModelData.qml"); // gives us 'tableView' variable

    // Generate javascript model
    const int size = 5;
    const char* modelUpdated = "modelUpdated";

    QJSEngine jsEngine;
    QJSValue jsArray;
    jsArray = jsEngine.newArray(size);
    for (int i = 0; i < size; ++i)
        jsArray.setProperty(i, QRandomGenerator::global()->generate());

    QVariant jsModel = QVariant::fromValue(jsArray);
    tableView->setModel(jsModel);
    WAIT_UNTIL_POLISHED;

    // Model change would be triggered for the first time
    QCOMPARE(tableView->property(modelUpdated).toInt(), 1);

    // Set the same model once again and check if model changes
    tableView->setModel(jsModel);
    QCOMPARE(tableView->property(modelUpdated).toInt(), 1);
}

QTEST_MAIN(tst_QQuickTableView)

#include "tst_qquicktableview.moc"
