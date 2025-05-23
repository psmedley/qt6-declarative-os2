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

#include <QtQuick/qquickview.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/private/qquickhoverhandler_p.h>
#include <QtQuick/private/qquickmousearea_p.h>
#include <qpa/qwindowsysteminterface.h>

#include <private/qquickwindow_p.h>

#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlproperty.h>

#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QtQuickTestUtils/private/viewtestutils_p.h>

Q_LOGGING_CATEGORY(lcPointerTests, "qt.quick.pointer.tests")

static bool isPlatformWayland()
{
    return !QGuiApplication::platformName().compare(QLatin1String("wayland"), Qt::CaseInsensitive);
}

class tst_HoverHandler : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_HoverHandler()
        : QQmlDataTest(QT_QMLTEST_DATADIR)
    {}

private slots:
    void hoverHandlerAndUnderlyingHoverHandler();
    void mouseAreaAndUnderlyingHoverHandler();
    void hoverHandlerAndUnderlyingMouseArea();
    void disabledHoverHandlerAndUnderlyingMouseArea();
    void movingItemWithHoverHandler();
    void margin();
    void window();
    void touchDrag();

private:
    void createView(QScopedPointer<QQuickView> &window, const char *fileName);

    QScopedPointer<QPointingDevice> touchscreen = QScopedPointer<QPointingDevice>(QTest::createTouchDevice());
};

void tst_HoverHandler::createView(QScopedPointer<QQuickView> &window, const char *fileName)
{
    window.reset(new QQuickView);
    window->setSource(testFileUrl(fileName));
    QTRY_COMPARE(window->status(), QQuickView::Ready);
    QQuickViewTestUtils::centerOnScreen(window.data());
    QQuickViewTestUtils::moveMouseAway(window.data());

    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window.data()));
    QVERIFY(window->rootObject() != nullptr);
}

void tst_HoverHandler::hoverHandlerAndUnderlyingHoverHandler()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "lesHoverables.qml");
    QQuickView * window = windowPtr.data();
    QQuickItem * topSidebar = window->rootObject()->findChild<QQuickItem *>("topSidebar");
    QVERIFY(topSidebar);
    QQuickItem * button = topSidebar->findChild<QQuickItem *>("buttonWithHH");
    QVERIFY(button);
    QQuickHoverHandler *topSidebarHH = topSidebar->findChild<QQuickHoverHandler *>("topSidebarHH");
    QVERIFY(topSidebarHH);
    QQuickHoverHandler *buttonHH = button->findChild<QQuickHoverHandler *>("buttonHH");
    QVERIFY(buttonHH);

    QPoint buttonCenter(button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint());
    QPoint rightOfButton(button->mapToScene(QPointF(button->width() + 2, button->height() / 2)).toPoint());
    QPoint outOfSidebar(topSidebar->mapToScene(QPointF(topSidebar->width() + 2, topSidebar->height() / 2)).toPoint());
    QSignalSpy sidebarHoveredSpy(topSidebarHH, SIGNAL(hoveredChanged()));
    QSignalSpy buttonHoveredSpy(buttonHH, SIGNAL(hoveredChanged()));

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(topSidebarHH->isHovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 0);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::ArrowCursor);
#endif

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(topSidebarHH->isHovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::OpenHandCursor);
#endif

    QTest::mouseMove(window, buttonCenter);
    QCOMPARE(topSidebarHH->isHovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonHH->isHovered(), true);
    QCOMPARE(buttonHoveredSpy.count(), 1);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::PointingHandCursor);
#endif

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(topSidebarHH->isHovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 2);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::OpenHandCursor);
#endif

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(topSidebarHH->isHovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 2);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 2);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::ArrowCursor);
#endif
}

void tst_HoverHandler::mouseAreaAndUnderlyingHoverHandler()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "lesHoverables.qml");
    QQuickView * window = windowPtr.data();
    QQuickItem * topSidebar = window->rootObject()->findChild<QQuickItem *>("topSidebar");
    QVERIFY(topSidebar);
    QQuickMouseArea * buttonMA = topSidebar->findChild<QQuickMouseArea *>("buttonMA");
    QVERIFY(buttonMA);
    QQuickHoverHandler *topSidebarHH = topSidebar->findChild<QQuickHoverHandler *>("topSidebarHH");
    QVERIFY(topSidebarHH);

    // Ensure that we don't get extra hover events delivered on the
    // side, since it can affect the number of hover move events we receive below.
    QQuickWindowPrivate::get(window)->deliveryAgentPrivate()->frameSynchronousHoverEnabled = false;
    // And flush out any mouse events that might be queued up
    // in QPA, since QTest::mouseMove() calls processEvents.
    qGuiApp->processEvents();

    QPoint buttonCenter(buttonMA->mapToScene(QPointF(buttonMA->width() / 2, buttonMA->height() / 2)).toPoint());
    QPoint rightOfButton(buttonMA->mapToScene(QPointF(buttonMA->width() + 2, buttonMA->height() / 2)).toPoint());
    QPoint outOfSidebar(topSidebar->mapToScene(QPointF(topSidebar->width() + 2, topSidebar->height() / 2)).toPoint());
    QSignalSpy sidebarHoveredSpy(topSidebarHH, SIGNAL(hoveredChanged()));
    QSignalSpy buttonHoveredSpy(buttonMA, SIGNAL(hoveredChanged()));

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(topSidebarHH->isHovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 0);
    QCOMPARE(buttonMA->hovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::ArrowCursor);
#endif

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(topSidebarHH->isHovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonMA->hovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::OpenHandCursor);
#endif

    QTest::mouseMove(window, buttonCenter);
    QCOMPARE(topSidebarHH->isHovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonMA->hovered(), true);
    QCOMPARE(buttonHoveredSpy.count(), 1);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::UpArrowCursor);
#endif

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(topSidebarHH->isHovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonMA->hovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 2);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::OpenHandCursor);
#endif

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(topSidebarHH->isHovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 2);
    QCOMPARE(buttonMA->hovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 2);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::ArrowCursor);
#endif
}

void tst_HoverHandler::hoverHandlerAndUnderlyingMouseArea()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "lesHoverables.qml");
    QQuickView * window = windowPtr.data();
    QQuickItem * bottomSidebar = window->rootObject()->findChild<QQuickItem *>("bottomSidebar");
    QVERIFY(bottomSidebar);
    QQuickMouseArea *bottomSidebarMA = bottomSidebar->findChild<QQuickMouseArea *>("bottomSidebarMA");
    QVERIFY(bottomSidebarMA);
    QQuickItem * button = bottomSidebar->findChild<QQuickItem *>("buttonWithHH");
    QVERIFY(button);
    QQuickHoverHandler *buttonHH = button->findChild<QQuickHoverHandler *>("buttonHH");
    QVERIFY(buttonHH);

    QPoint buttonCenter(button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint());
    QPoint rightOfButton(button->mapToScene(QPointF(button->width() + 2, button->height() / 2)).toPoint());
    QPoint outOfSidebar(bottomSidebar->mapToScene(QPointF(bottomSidebar->width() + 2, bottomSidebar->height() / 2)).toPoint());
    QSignalSpy sidebarHoveredSpy(bottomSidebarMA, SIGNAL(hoveredChanged()));
    QSignalSpy buttonHoveredSpy(buttonHH, SIGNAL(hoveredChanged()));

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(bottomSidebarMA->hovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 0);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::ArrowCursor);
#endif

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(bottomSidebarMA->hovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::ClosedHandCursor);
#endif

    QTest::mouseMove(window, buttonCenter);
    QCOMPARE(bottomSidebarMA->hovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 2);
    QCOMPARE(buttonHH->isHovered(), true);
    QCOMPARE(buttonHoveredSpy.count(), 1);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::PointingHandCursor);
#endif

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(bottomSidebarMA->hovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 3);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 2);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::ClosedHandCursor);
#endif

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(bottomSidebarMA->hovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 4);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 2);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::ArrowCursor);
#endif
}

void tst_HoverHandler::disabledHoverHandlerAndUnderlyingMouseArea()
{
    // Check that if a disabled HoverHandler is installed on an item, it
    // will not participate in hover event delivery, and as such, also
    // not block propagation to siblings.
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "lesHoverables.qml");
    QQuickView * window = windowPtr.data();
    QQuickItem * bottomSidebar = window->rootObject()->findChild<QQuickItem *>("bottomSidebar");
    QVERIFY(bottomSidebar);
    QQuickMouseArea *bottomSidebarMA = bottomSidebar->findChild<QQuickMouseArea *>("bottomSidebarMA");
    QVERIFY(bottomSidebarMA);
    QQuickItem * button = bottomSidebar->findChild<QQuickItem *>("buttonWithHH");
    QVERIFY(button);
    QQuickHoverHandler *buttonHH = button->findChild<QQuickHoverHandler *>("buttonHH");
    QVERIFY(buttonHH);

    // By disabling the HoverHandler, it should no longer
    // block the sibling MouseArea underneath from receiving hover events.
    buttonHH->setEnabled(false);

    QPoint buttonCenter(button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint());
    QPoint rightOfButton(button->mapToScene(QPointF(button->width() + 2, button->height() / 2)).toPoint());
    QPoint outOfSidebar(bottomSidebar->mapToScene(QPointF(bottomSidebar->width() + 2, bottomSidebar->height() / 2)).toPoint());
    QSignalSpy sidebarHoveredSpy(bottomSidebarMA, SIGNAL(hoveredChanged()));
    QSignalSpy buttonHoveredSpy(buttonHH, SIGNAL(hoveredChanged()));

    QTest::mouseMove(window, outOfSidebar);
    QCOMPARE(bottomSidebarMA->hovered(), false);
    QCOMPARE(sidebarHoveredSpy.count(), 0);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);

    QTest::mouseMove(window, buttonCenter);
    QCOMPARE(bottomSidebarMA->hovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);

    QTest::mouseMove(window, rightOfButton);
    QCOMPARE(bottomSidebarMA->hovered(), true);
    QCOMPARE(sidebarHoveredSpy.count(), 1);
    QCOMPARE(buttonHH->isHovered(), false);
    QCOMPARE(buttonHoveredSpy.count(), 0);
}

void tst_HoverHandler::movingItemWithHoverHandler()
{
   if (isPlatformWayland())
        QSKIP("Wayland: QCursor::setPos() doesn't work.");

    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "lesHoverables.qml");
    QQuickView * window = windowPtr.data();
    QQuickItem * paddle = window->rootObject()->findChild<QQuickItem *>("paddle");
    QVERIFY(paddle);
    QQuickHoverHandler *paddleHH = paddle->findChild<QQuickHoverHandler *>("paddleHH");
    QVERIFY(paddleHH);

    // Find the global coordinate of the paddle
    const QPoint p(paddle->mapToScene(paddle->clipRect().center()).toPoint());
    const QPoint paddlePos = window->mapToGlobal(p);

    // Now hide the window, put the cursor where the paddle was and show it again
    window->hide();
    QTRY_COMPARE(window->isVisible(), false);
    QCursor::setPos(paddlePos);
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QTRY_COMPARE(paddleHH->isHovered(), true);
    // TODO check the cursor shape after fixing QTBUG-53987

    paddle->setX(100);
    QTRY_COMPARE(paddleHH->isHovered(), false);

    paddle->setX(p.x() - paddle->width() / 2);
    QTRY_COMPARE(paddleHH->isHovered(), true);

    paddle->setX(540);
    QTRY_COMPARE(paddleHH->isHovered(), false);
}

void tst_HoverHandler::margin() // QTBUG-85303
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "hoverMargin.qml");
    QQuickView * window = windowPtr.data();
    QQuickItem * item = window->rootObject()->findChild<QQuickItem *>();
    QVERIFY(item);
    QQuickHoverHandler *hh = item->findChild<QQuickHoverHandler *>();
    QVERIFY(hh);

    QPoint itemCenter(item->mapToScene(QPointF(item->width() / 2, item->height() / 2)).toPoint());
    QPoint leftMargin = itemCenter - QPoint(35, 35);
    QSignalSpy hoveredSpy(hh, SIGNAL(hoveredChanged()));

    QTest::mouseMove(window, {10, 10});
    QCOMPARE(hh->isHovered(), false);
    QCOMPARE(hoveredSpy.count(), 0);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::ArrowCursor);
#endif

    QTest::mouseMove(window, leftMargin);
    QCOMPARE(hh->isHovered(), true);
    QCOMPARE(hoveredSpy.count(), 1);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::OpenHandCursor);
#endif

    QTest::mouseMove(window, itemCenter);
    QCOMPARE(hh->isHovered(), true);
    QCOMPARE(hoveredSpy.count(), 1);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::OpenHandCursor);
#endif

    QTest::mouseMove(window, leftMargin);
    QCOMPARE(hh->isHovered(), true);
//    QCOMPARE(hoveredSpy.count(), 1);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::OpenHandCursor);
#endif

    QTest::mouseMove(window, {10, 10});
    QCOMPARE(hh->isHovered(), false);
//    QCOMPARE(hoveredSpy.count(), 2);
#if QT_CONFIG(cursor)
    QCOMPARE(window->cursor().shape(), Qt::ArrowCursor);
#endif
}

void tst_HoverHandler::window() // QTBUG-98717
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("windowCursorShape.qml"));
    QScopedPointer<QQuickWindow> window(qobject_cast<QQuickWindow *>(component.create()));
    QVERIFY(!window.isNull());
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window.data()));
#if QT_CONFIG(cursor)
    if (isPlatformWayland())
         QSKIP("Wayland: QCursor::setPos() doesn't work.");
    auto cursorPos = window->mapToGlobal(QPoint(100, 100));
    qCDebug(lcPointerTests) << "in window @" << window->position() << "setting cursor pos" << cursorPos;
    QCursor::setPos(cursorPos);
    if (!QTest::qWaitFor([cursorPos]{ return QCursor::pos() == cursorPos; }))
        QSKIP("QCursor::setPos() doesn't work (QTBUG-76312).");
    QTRY_COMPARE(window->cursor().shape(), Qt::OpenHandCursor);
#endif
}

void tst_HoverHandler::touchDrag()
{
    QQuickView window;
    QVERIFY(QQuickTest::showView(window, testFileUrl("hoverHandler.qml")));
    const QQuickItem *root = window.rootObject();
    QQuickHoverHandler *handler = root->findChild<QQuickHoverHandler *>();
    QVERIFY(handler);

    // polishAndSync() calls flushFrameSynchronousEvents() before emitting afterAnimating()
    QSignalSpy frameSyncSpy(&window, &QQuickWindow::afterAnimating);

    const QPoint out(root->width() - 1, root->height() / 2);
    QPoint in(root->width() / 2, root->height() / 2);

    QTest::touchEvent(&window, touchscreen.get()).press(0, out, &window);
    QQuickTouchUtils::flush(&window);
    QCOMPARE(handler->isHovered(), false);

    frameSyncSpy.clear();
    QTest::touchEvent(&window, touchscreen.get()).move(0, in, &window);
    QQuickTouchUtils::flush(&window);
    QTRY_COMPARE(handler->isHovered(), true);
    QCOMPARE(handler->point().scenePosition(), in);

    in += {10, 10};
    QTest::touchEvent(&window, touchscreen.get()).move(0, in, &window);
    QQuickTouchUtils::flush(&window);
    // ensure that the color change is visible
    QTRY_VERIFY(frameSyncSpy.size() >= 1);
    QCOMPARE(handler->isHovered(), true);
    QCOMPARE(handler->point().scenePosition(), in);

    QTest::touchEvent(&window, touchscreen.get()).move(0, out, &window);
    QQuickTouchUtils::flush(&window);
    QTRY_VERIFY(frameSyncSpy.size() >= 2);
    QCOMPARE(handler->isHovered(), false);

    QTest::touchEvent(&window, touchscreen.get()).release(0, out, &window);
}

QTEST_MAIN(tst_HoverHandler)

#include "tst_qquickhoverhandler.moc"
