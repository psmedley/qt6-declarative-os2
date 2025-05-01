// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>
#include <QtTest/qsignalspy.h>
#include <QtQuickTest/quicktest.h>

#include <QtCore/qoperatingsystemversion.h>
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QtGui/qpa/qplatformintegration.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtQuick/qquickview.h>
#include <QtQuick/private/qquickmousearea_p.h>
#include <QtQuick/private/qquickpalette_p.h>
#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QtQuickTestUtils/private/viewtestutils_p.h>
#include <QtQuickTestUtils/private/visualtestutils_p.h>
#include <QtQuickTemplates2/private/qquickapplicationwindow_p.h>
#include <QtQuickTemplates2/private/qquickbutton_p.h>
#include <QtQuickTemplates2/private/qquickcombobox_p.h>
#include <QtQuickTemplates2/private/qquickdialog_p.h>
#include <QtQuickTemplates2/private/qquickdrawer_p.h>
#include <QtQuickTemplates2/private/qquickoverlay_p.h>
#include <QtQuickTemplates2/private/qquickoverlay_p_p.h>
#include <QtQuickTemplates2/private/qquickpopup_p.h>
#include <QtQuickTemplates2/private/qquickpopup_p_p.h>
#include <QtQuickTemplates2/private/qquickpopupanchors_p.h>
#include <QtQuickTemplates2/private/qquickpopupitem_p_p.h>
#include <QtQuickTemplates2/private/qquickpopupwindow_p_p.h>
#include <QtQuickTemplates2/private/qquickslider_p.h>
#include <QtQuickTemplates2/private/qquickstackview_p.h>
#include <QtQuickTemplates2/private/qquicktooltip_p.h>
#include <QtQuick/private/qquicktextinput_p.h>
#include <QtQuick/private/qquicklistview_p.h>
#include <QtQuick/private/qquicktextedit_p.h>
#include <QtQuick/private/qquickdroparea_p.h>
#include <QtQuickControlsTestUtils/private/controlstestutils_p.h>
#include <QtQuickControlsTestUtils/private/qtest_quickcontrols_p.h>

using namespace QQuickVisualTestUtils;
using namespace QQuickControlsTestUtils;

class tst_QQuickPopup : public QQmlDataTest
{
    Q_OBJECT

public:
    tst_QQuickPopup();

private slots:
    void cleanup();
    void initTestCase() override;
    void visible_data();
    void visible();
    void state();
    void overlay_data();
    void overlay();
    void zOrder_data();
    void zOrder();
    void windowChange();
    void closePolicy_data();
    void closePolicy();
    void closePolicy_grabberInside_data();
    void closePolicy_grabberInside();
    void activeFocusOnClose1();
    void activeFocusOnClose2();
    void activeFocusOnClose3();
    void activeFocusOnClosingSeveralPopups();
    void activeFocusAfterExit();
    void activeFocusOnDelayedEnter();
    void activeFocusDespiteLowerStackingOrder();
    void activeFocusItemAfterWindowInactive();
    void hover_data();
    void hover();
#if QT_CONFIG(wheelevent)
    void wheel_data();
    void wheel();
#endif
    void parentDestroyed();
    void nested();
#if QT_CONFIG(wheelevent)
    void nestedWheel();
    void nestedWheelWithOverlayParent();
#endif
    void modelessOnModalOnModeless();
    void grabber();
    void cursorShape();
    void componentComplete();
    void closeOnEscapeWithNestedPopups();
    void closeOnEscapeWithVisiblePopup();
    void enabled();
    void orientation_data();
    void orientation();
    void qquickview();
    void disabledPalette();
    void disabledParentPalette();
    void countChanged();
    void toolTipCrashOnClose();
    void setOverlayParentToNull();
    void tabFence();
    void invisibleToolTipOpen();
    void centerInOverlayWithinStackViewItem();
    void destroyDuringExitTransition();
    void releaseAfterExitTransition();
    void dimmerContainmentMask();
    void shrinkPopupThatWasLargerThanWindow_data();
    void shrinkPopupThatWasLargerThanWindow();
    void relativeZOrder();
    void mirroredCombobox();
    void rotatedCombobox();
    void focusMultiplePopup();
    void contentChildrenChange();
    void doubleClickInMouseArea();
    void fadeDimmer_data();
    void fadeDimmer();
    void noDimmer();
    void pointerEventsNotBlockedForNonPopupChildrenOfOverlayWithHigherZ();

    void popupWindowPositioning();
    void popupWindowAnchorsCenterIn_data();
    void popupWindowAnchorsCenterIn();
    void popupWindowModality();
    void popupWindowClosesOnParentWindowClosing();
    void popupWindowClosingPolicy();
    void initialPopupSize_data();
    void initialPopupSize();
    void popupWindowChangingParent();
    void popupWindowChangingParentWindow();
    void popupWindowFocus();
    void popupTypeChangeFromWindowToItem();
    void popupTypeChangeFromItemToWindow();
    void resetHoveredStateForItemsWithinPopup();
    void noInfiniteRecursionOnParentWindowDestruction();
    void popupWindowDestructedBeforeQQuickPopup();
    void popupWindowPositionerRespectingScreenBounds_data();
    void popupWindowPositionerRespectingScreenBounds();

private:
    QScopedPointer<QPointingDevice> touchScreen = QScopedPointer<QPointingDevice>(QTest::createTouchDevice());
    bool popupWindowsSupported = false;
};

using namespace Qt::StringLiterals;

tst_QQuickPopup::tst_QQuickPopup()
    : QQmlDataTest(QT_QMLTEST_DATADIR)
{
#if defined(Q_OS_WINDOWS) || defined(Q_OS_MACOS)
    popupWindowsSupported = QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::Capability::MultipleWindows);
#endif
}

void tst_QQuickPopup::cleanup()
{
    // For some reason, it's not impossible for popups to already exist, when a test is executed.
    if (QGuiApplicationPrivate::popupCount() > 0)
        QGuiApplicationPrivate::closeAllPopups();
    QTRY_COMPARE(QGuiApplicationPrivate::popupCount(), 0);
}

void tst_QQuickPopup::initTestCase()
{
    QQmlDataTest::initTestCase();
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuWindows);
    qputenv("QML_NO_TOUCH_COMPRESSION", "1");
}

void tst_QQuickPopup::visible_data()
{
    QTest::addColumn<QString>("source");
    QTest::newRow("Window") << "window.qml";
    QTest::newRow("ApplicationWindow") << "applicationwindow.qml";
}

void tst_QQuickPopup::visible()
{
    QFETCH(QString, source);
    QQuickControlsApplicationHelper helper(this, source);
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *popup = window->property("popup").value<QQuickPopup*>();
    QVERIFY(popup);
    QQuickItem *popupItem = popup->popupItem();
    popup->setPopupType(QQuickPopup::Item);

    popup->open();
    QTRY_VERIFY(popup->isOpened());

    QQuickOverlay *overlay = QQuickOverlay::overlay(window);
    QVERIFY(overlay);
    QVERIFY(overlay->childItems().contains(popupItem));

    popup->close();
    QTRY_VERIFY(!popup->isVisible());
    QVERIFY(!overlay->childItems().contains(popupItem));

    popup->setVisible(true);
    QTRY_VERIFY(popup->isOpened());
    QVERIFY(overlay->childItems().contains(popupItem));

    popup->setVisible(false);
    QTRY_VERIFY(!popup->isVisible());
    QVERIFY(!overlay->childItems().contains(popupItem));
}

void tst_QQuickPopup::state()
{
    QQuickControlsApplicationHelper helper(this, "applicationwindow.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *popup = window->property("popup").value<QQuickPopup*>();
    QVERIFY(popup);

    QCOMPARE(popup->isVisible(), false);

    QSignalSpy visibleChangedSpy(popup, SIGNAL(visibleChanged()));
    QSignalSpy aboutToShowSpy(popup, SIGNAL(aboutToShow()));
    QSignalSpy aboutToHideSpy(popup, SIGNAL(aboutToHide()));
    QSignalSpy openedSpy(popup, SIGNAL(opened()));
    QSignalSpy closedSpy(popup, SIGNAL(closed()));

    QVERIFY(visibleChangedSpy.isValid());
    QVERIFY(aboutToShowSpy.isValid());
    QVERIFY(aboutToHideSpy.isValid());
    QVERIFY(openedSpy.isValid());
    QVERIFY(closedSpy.isValid());

    popup->open();
    QCOMPARE(visibleChangedSpy.size(), 1);
    QCOMPARE(aboutToShowSpy.size(), 1);
    QCOMPARE(aboutToHideSpy.size(), 0);
    QTRY_COMPARE(openedSpy.size(), 1);
    QCOMPARE(closedSpy.size(), 0);

    popup->close();
    QTRY_COMPARE(visibleChangedSpy.size(), 2);
    QCOMPARE(aboutToShowSpy.size(), 1);
    QCOMPARE(aboutToHideSpy.size(), 1);
    QCOMPARE(openedSpy.size(), 1);
    QTRY_COMPARE(closedSpy.size(), 1);
}

void tst_QQuickPopup::overlay_data()
{
    QTest::addColumn<QString>("source");
    QTest::addColumn<bool>("modal");
    QTest::addColumn<bool>("dim");

    QTest::newRow("Window") << "window.qml" << false << false;
    QTest::newRow("Window,dim") << "window.qml" << false << true;
    QTest::newRow("Window,modal") << "window.qml" << true << false;
    QTest::newRow("Window,modal,dim") << "window.qml" << true << true;

    QTest::newRow("ApplicationWindow") << "applicationwindow.qml" << false << false;
    QTest::newRow("ApplicationWindow,dim") << "applicationwindow.qml" << false << true;
    QTest::newRow("ApplicationWindow,modal") << "applicationwindow.qml" << true << false;
    QTest::newRow("ApplicationWindow,modal,dim") << "applicationwindow.qml" << true << true;
}

void tst_QQuickPopup::overlay()
{
#ifdef Q_OS_ANDROID
    QSKIP("Test crashes. See QTBUG-118532");
#endif

    QFETCH(QString, source);
    QFETCH(bool, modal);
    QFETCH(bool, dim);

    QQuickControlsApplicationHelper helper(this, source);
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickOverlay *overlay = QQuickOverlay::overlay(window);
    QVERIFY(overlay);

    QSignalSpy overlayPressedSignal(overlay, SIGNAL(pressed()));
    QSignalSpy overlayReleasedSignal(overlay, SIGNAL(released()));
    QVERIFY(overlayPressedSignal.isValid());
    QVERIFY(overlayReleasedSignal.isValid());

    QVERIFY(!overlay->isVisible()); // no popups open

    QTest::mouseClick(window, Qt::LeftButton);
    QCOMPARE(overlayPressedSignal.size(), 0);
    QCOMPARE(overlayReleasedSignal.size(), 0);

    QQuickPopup *popup = window->property("popup").value<QQuickPopup*>();
    QVERIFY(popup);
    popup->setPopupType(QQuickPopup::Item);

    QQuickOverlayAttached *overlayAttached = qobject_cast<QQuickOverlayAttached *>(qmlAttachedPropertiesObject<QQuickOverlay>(popup));
    QVERIFY(overlayAttached);
    QCOMPARE(overlayAttached->overlay(), overlay);

    QSignalSpy overlayAttachedPressedSignal(overlayAttached, SIGNAL(pressed()));
    QSignalSpy overlayAttachedReleasedSignal(overlayAttached, SIGNAL(released()));
    QVERIFY(overlayAttachedPressedSignal.isValid());
    QVERIFY(overlayAttachedReleasedSignal.isValid());

    QQuickButton *button = window->property("button").value<QQuickButton*>();
    QVERIFY(button);

    int overlayPressCount = 0;
    int overlayReleaseCount = 0;

    popup->open();
    QVERIFY(popup->isVisible());
    QVERIFY(overlay->isVisible());
    QTRY_VERIFY(popup->isOpened());

    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QCOMPARE(overlayPressedSignal.size(), ++overlayPressCount);
    QCOMPARE(overlayReleasedSignal.size(), overlayReleaseCount);
    QCOMPARE(overlayAttachedPressedSignal.size(), overlayPressCount);
    QCOMPARE(overlayAttachedReleasedSignal.size(), overlayReleaseCount);

    QTRY_VERIFY(!popup->isVisible());
    QVERIFY(!overlay->isVisible());

    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QCOMPARE(overlayPressedSignal.size(), overlayPressCount);
    QCOMPARE(overlayReleasedSignal.size(), overlayReleaseCount); // no modal-popups open
    QCOMPARE(overlayAttachedPressedSignal.size(), overlayPressCount);
    QCOMPARE(overlayAttachedReleasedSignal.size(), overlayReleaseCount);

    popup->setDim(dim);
    popup->setModal(modal);
    popup->setClosePolicy(QQuickPopup::CloseOnReleaseOutside);

    // mouse
    popup->open();
    QVERIFY(popup->isVisible());
    QVERIFY(overlay->isVisible());
    QTRY_VERIFY(popup->isOpened());

    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QCOMPARE(overlayPressedSignal.size(), ++overlayPressCount);
    QCOMPARE(overlayReleasedSignal.size(), overlayReleaseCount);
    QCOMPARE(overlayAttachedPressedSignal.size(), overlayPressCount);
    QCOMPARE(overlayAttachedReleasedSignal.size(), overlayReleaseCount);

    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QCOMPARE(overlayPressedSignal.size(), overlayPressCount);
    QCOMPARE(overlayReleasedSignal.size(), ++overlayReleaseCount);
    QCOMPARE(overlayAttachedPressedSignal.size(), overlayPressCount);
    QCOMPARE(overlayAttachedReleasedSignal.size(), overlayReleaseCount);

    QTRY_VERIFY(!popup->isVisible());
    QVERIFY(!overlay->isVisible());

    // touch
    popup->open();
    QVERIFY(popup->isVisible());
    QVERIFY(overlay->isVisible());
    QTRY_VERIFY(popup->isOpened());

    QTest::touchEvent(window, touchScreen.data()).press(0, QPoint(1, 1));
    QCOMPARE(overlayPressedSignal.size(), ++overlayPressCount);
    QCOMPARE(overlayReleasedSignal.size(), overlayReleaseCount);
    QCOMPARE(overlayAttachedPressedSignal.size(), overlayPressCount);
    QCOMPARE(overlayAttachedReleasedSignal.size(), overlayReleaseCount);

    QTest::touchEvent(window, touchScreen.data()).release(0, QPoint(1, 1));
    QCOMPARE(overlayPressedSignal.size(), overlayPressCount);
    QCOMPARE(overlayReleasedSignal.size(), ++overlayReleaseCount);
    QCOMPARE(overlayAttachedPressedSignal.size(), overlayPressCount);
    QCOMPARE(overlayAttachedReleasedSignal.size(), overlayReleaseCount);

    QTRY_VERIFY(!popup->isVisible());
    QVERIFY(!overlay->isVisible());

    // multi-touch
    popup->open();
    QVERIFY(popup->isVisible());
    QVERIFY(overlay->isVisible());
    QVERIFY(!button->isPressed());
    QTRY_VERIFY(popup->isOpened());

    QTest::touchEvent(window, touchScreen.data()).press(0, button->mapToScene(QPointF(1, 1)).toPoint());
    QVERIFY(popup->isVisible());
    QVERIFY(overlay->isVisible());
    QCOMPARE(button->isPressed(), !modal);
    QCOMPARE(overlayPressedSignal.size(), ++overlayPressCount);
    QCOMPARE(overlayReleasedSignal.size(), overlayReleaseCount);

    QTest::touchEvent(window, touchScreen.data()).stationary(0).press(1, button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint());
    QVERIFY(popup->isVisible());
    QVERIFY(overlay->isVisible());
    QCOMPARE(button->isPressed(), !modal);
    QCOMPARE(overlayPressedSignal.size(), ++overlayPressCount);
    QCOMPARE(overlayReleasedSignal.size(), overlayReleaseCount);

    QTest::touchEvent(window, touchScreen.data()).release(0, button->mapToScene(QPointF(1, 1)).toPoint()).stationary(1);
    QTRY_VERIFY(!popup->isVisible());
    QVERIFY(!overlay->isVisible());
    QVERIFY(!button->isPressed());
    QCOMPARE(overlayPressedSignal.size(), overlayPressCount);
    QCOMPARE(overlayReleasedSignal.size(), ++overlayReleaseCount);

    QTest::touchEvent(window, touchScreen.data()).release(1, button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint());
    QVERIFY(!popup->isVisible());
    QVERIFY(!overlay->isVisible());
    QVERIFY(!button->isPressed());
    QCOMPARE(overlayPressedSignal.size(), overlayPressCount);
    QCOMPARE(overlayReleasedSignal.size(), overlayReleaseCount);
}

void tst_QQuickPopup::zOrder_data()
{
    QTest::addColumn<QString>("source");
    QTest::newRow("Window") << "window.qml";
    QTest::newRow("ApplicationWindow") << "applicationwindow.qml";
}

void tst_QQuickPopup::zOrder()
{
    QFETCH(QString, source);
    QQuickControlsApplicationHelper helper(this, source);
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *popup = window->property("popup").value<QQuickPopup*>();
    QVERIFY(popup);
    popup->setPopupType(QQuickPopup::Item);
    popup->setModal(true);

    QQuickPopup *popup2 = window->property("popup2").value<QQuickPopup*>();
    QVERIFY(popup2);
    popup2->setPopupType(QQuickPopup::Item);
    popup2->setModal(true);

    // show popups in reverse order. popup2 has higher z-order so it appears
    // on top and must be closed first, even if the other popup was opened last
    popup2->open();
    popup->open();
    QTRY_VERIFY(popup2->isOpened());
    QTRY_VERIFY(popup->isOpened());

    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QTRY_VERIFY(!popup2->isVisible());
    QVERIFY(popup->isVisible());

    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QVERIFY(!popup2->isVisible());
    QTRY_VERIFY(!popup->isVisible());
}

void tst_QQuickPopup::windowChange()
{
    QQuickPopup popup;
    QSignalSpy spy(&popup, SIGNAL(windowChanged(QQuickWindow*)));
    QVERIFY(spy.isValid());

    QQuickItem item;
    popup.setParentItem(&item);
    QVERIFY(!popup.window());
    QCOMPARE(spy.size(), 0);

    QQuickWindow window;
    item.setParentItem(window.contentItem());
    QCOMPARE(popup.window(), &window);
    QCOMPARE(spy.size(), 1);

    item.setParentItem(nullptr);
    QVERIFY(!popup.window());
    QCOMPARE(spy.size(), 2);

    popup.setParentItem(window.contentItem());
    QCOMPARE(popup.window(), &window);
    QCOMPARE(spy.size(), 3);

    popup.resetParentItem();
    QVERIFY(!popup.window());
    QCOMPARE(spy.size(), 4);

    popup.setParent(&window);
    popup.resetParentItem();
    QCOMPARE(popup.window(), &window);
    QCOMPARE(spy.size(), 5);

    popup.setParent(this);
    popup.resetParentItem();
    QVERIFY(!popup.window());
    QCOMPARE(spy.size(), 6);

    item.setParentItem(window.contentItem());
    popup.setParent(&item);
    popup.resetParentItem();
    QCOMPARE(popup.window(), &window);
    QCOMPARE(spy.size(), 7);

    popup.setParent(nullptr);
}

Q_DECLARE_METATYPE(QQuickPopup::ClosePolicy)

void tst_QQuickPopup::closePolicy_data()
{
    qRegisterMetaType<QQuickPopup::ClosePolicy>();
    const auto *mouse = QPointingDevice::primaryPointingDevice();
    const auto *touch = touchScreen.data();

    QTest::addColumn<QString>("source");
    QTest::addColumn<const QPointingDevice *>("device");
    QTest::addColumn<QQuickPopup::ClosePolicy>("closePolicy");

    QTest::newRow("Window:NoAutoClose mouse") << "window.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::NoAutoClose);
    QTest::newRow("Window:CloseOnPressOutside mouse") << "window.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnPressOutside);
    QTest::newRow("Window:CloseOnPressOutsideParent mouse") << "window.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnPressOutsideParent);
    QTest::newRow("Window:CloseOnPressOutside|Parent mouse") << "window.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnPressOutside | QQuickPopup::CloseOnPressOutsideParent);
    QTest::newRow("Window:CloseOnReleaseOutside mouse") << "window.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnReleaseOutside);
    QTest::newRow("Window:CloseOnReleaseOutside|Parent mouse") << "window.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnReleaseOutside | QQuickPopup::CloseOnReleaseOutsideParent);
    QTest::newRow("Window:CloseOnEscape mouse") << "window.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnEscape);

    QTest::newRow("ApplicationWindow:NoAutoClose mouse") << "applicationwindow.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::NoAutoClose);
    QTest::newRow("ApplicationWindow:CloseOnPressOutside mouse") << "applicationwindow.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnPressOutside);
    QTest::newRow("ApplicationWindow:CloseOnPressOutsideParent mouse") << "applicationwindow.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnPressOutsideParent);
    QTest::newRow("ApplicationWindow:CloseOnPressOutside|Parent mouse") << "applicationwindow.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnPressOutside | QQuickPopup::CloseOnPressOutsideParent);
    QTest::newRow("ApplicationWindow:CloseOnReleaseOutside mouse") << "applicationwindow.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnReleaseOutside);
    QTest::newRow("ApplicationWindow:CloseOnReleaseOutside|Parent mouse") << "applicationwindow.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnReleaseOutside | QQuickPopup::CloseOnReleaseOutsideParent);
    QTest::newRow("ApplicationWindow:CloseOnEscape mouse") << "applicationwindow.qml" << mouse << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnEscape);

    QTest::newRow("Window:NoAutoClose touch") << "window.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::NoAutoClose);
    QTest::newRow("Window:CloseOnPressOutside touch") << "window.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnPressOutside);
    QTest::newRow("Window:CloseOnPressOutsideParent touch") << "window.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnPressOutsideParent);
    QTest::newRow("Window:CloseOnPressOutside|Parent touch") << "window.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnPressOutside | QQuickPopup::CloseOnPressOutsideParent);
    QTest::newRow("Window:CloseOnReleaseOutside touch") << "window.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnReleaseOutside);
    QTest::newRow("Window:CloseOnReleaseOutside|Parent touch") << "window.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnReleaseOutside | QQuickPopup::CloseOnReleaseOutsideParent);
    QTest::newRow("Window:CloseOnEscape touch") << "window.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnEscape);

    QTest::newRow("ApplicationWindow:NoAutoClose touch") << "applicationwindow.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::NoAutoClose);
    QTest::newRow("ApplicationWindow:CloseOnPressOutside touch") << "applicationwindow.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnPressOutside);
    QTest::newRow("ApplicationWindow:CloseOnPressOutsideParent touch") << "applicationwindow.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnPressOutsideParent);
    QTest::newRow("ApplicationWindow:CloseOnPressOutside|Parent touch") << "applicationwindow.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnPressOutside | QQuickPopup::CloseOnPressOutsideParent);
    QTest::newRow("ApplicationWindow:CloseOnReleaseOutside touch") << "applicationwindow.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnReleaseOutside);
    QTest::newRow("ApplicationWindow:CloseOnReleaseOutside|Parent touch") << "applicationwindow.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnReleaseOutside | QQuickPopup::CloseOnReleaseOutsideParent);
    QTest::newRow("ApplicationWindow:CloseOnEscape touch") << "applicationwindow.qml" << touch << static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnEscape);
}

void tst_QQuickPopup::closePolicy()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    QFETCH(QString, source);
    QFETCH(const QPointingDevice *, device);
    QFETCH(QQuickPopup::ClosePolicy, closePolicy);

    QQuickControlsApplicationHelper helper(this, source);
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickPopup *popup = window->property("popup").value<QQuickPopup*>();
    QVERIFY(popup);

    QQuickButton *button = window->property("button").value<QQuickButton*>();
    QVERIFY(button);

    popup->setPopupType(QQuickPopup::Item);
    popup->setModal(true);
    popup->setFocus(true);
    popup->setClosePolicy(closePolicy);

    popup->open();
    QVERIFY(popup->isVisible());
    QTRY_VERIFY(popup->isOpened());

    // wait for dimmer
    QTest::qWait(50);

    for (int i = 0; i < 2; ++i) {
        // press outside popup and its parent
        QQuickTest::pointerPress(device, window, 0, {1, 1});
        if (closePolicy.testFlag(QQuickPopup::CloseOnPressOutside) || closePolicy.testFlag(QQuickPopup::CloseOnPressOutsideParent))
            QTRY_VERIFY(!popup->isVisible());
        else
            QVERIFY(popup->isOpened());

        popup->open();
        QVERIFY(popup->isVisible());
        QTRY_VERIFY(popup->isOpened());

        // release outside popup and its parent
        QQuickTest::pointerRelease(device, window, 0, {1, 1});
        if (closePolicy.testFlag(QQuickPopup::CloseOnReleaseOutside) || closePolicy.testFlag(QQuickPopup::CloseOnReleaseOutsideParent))
            QTRY_VERIFY(!popup->isVisible());
        else
            QVERIFY(popup->isOpened());

        popup->open();
        QVERIFY(popup->isVisible());
        QTRY_VERIFY(popup->isOpened());

        // press outside popup but inside its parent
        QQuickTest::pointerPress(device, window, 0, QPoint(button->x() + 1, button->y() + 1));
        if (closePolicy.testFlag(QQuickPopup::CloseOnPressOutside) && !closePolicy.testFlag(QQuickPopup::CloseOnPressOutsideParent))
            QTRY_VERIFY(!popup->isVisible());
        else
            QVERIFY(popup->isOpened());

        popup->open();
        QVERIFY(popup->isVisible());
        QTRY_VERIFY(popup->isOpened());

        // release outside popup but inside its parent
        QQuickTest::pointerRelease(device, window, 0, QPoint(button->x() + 1, button->y() + 1));
        if (closePolicy.testFlag(QQuickPopup::CloseOnReleaseOutside) && !closePolicy.testFlag(QQuickPopup::CloseOnReleaseOutsideParent))
            QTRY_VERIFY(!popup->isVisible());
        else
            QVERIFY(popup->isOpened());

        popup->open();
        QVERIFY(popup->isVisible());
        QTRY_VERIFY(popup->isOpened());

        // press inside and release outside
        QQuickTest::pointerPress(device, window, 0, QPoint(button->x() + popup->x() + 1,
                                                           button->y() + popup->y() + 1));
        QVERIFY(popup->isOpened());
        QQuickTest::pointerRelease(device, window, 0, {1, 1});
        QVERIFY(popup->isOpened());
    }

    // escape
    QTest::keyClick(window, Qt::Key_Escape);
    if (closePolicy.testFlag(QQuickPopup::CloseOnEscape))
        QTRY_VERIFY(!popup->isVisible());
    else
        QVERIFY(popup->isVisible());
}

void tst_QQuickPopup::closePolicy_grabberInside_data()
{
    qRegisterMetaType<QQuickPopup::ClosePolicy>();

    QTest::addColumn<QString>("source");
    QTest::addColumn<QQuickPopup::ClosePolicy>("closePolicy");

    QTest::newRow("Window:CloseOnReleaseOutside") << "window.qml"<< static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnReleaseOutside);
    QTest::newRow("Window:CloseOnReleaseOutside|Parent") << "window.qml"<< static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnReleaseOutside | QQuickPopup::CloseOnReleaseOutsideParent);

    QTest::newRow("ApplicationWindow:CloseOnReleaseOutside") << "applicationwindow.qml"<< static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnReleaseOutside);
    QTest::newRow("ApplicationWindow:CloseOnReleaseOutside|Parent") << "applicationwindow.qml"<< static_cast<QQuickPopup::ClosePolicy>(QQuickPopup::CloseOnReleaseOutside | QQuickPopup::CloseOnReleaseOutsideParent);
}

void tst_QQuickPopup::closePolicy_grabberInside()
{
    QFETCH(QString, source);
    QFETCH(QQuickPopup::ClosePolicy, closePolicy);

    QQuickControlsApplicationHelper helper(this, source);
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *popup = window->property("popup3").value<QQuickPopup*>();
    QVERIFY(popup);

    QQuickSlider *slider = window->property("slider").value<QQuickSlider*>();
    QVERIFY(slider);

    popup->setModal(true);
    popup->setClosePolicy(closePolicy);

    popup->open();
    QVERIFY(popup->isVisible());
    QTRY_VERIFY(popup->isOpened());

    // press on a mouse grabber inside and release outside
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier,
                      slider->handle()->mapToItem(window->contentItem(),slider->handle()->boundingRect().center()).toPoint());

    QVERIFY(popup->isOpened());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QVERIFY(popup->isOpened());
}

void tst_QQuickPopup::activeFocusOnClose1()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    // Test that a popup that never sets focus: true (e.g. ToolTip) doesn't affect
    // the active focus item when it closes.
    QQuickControlsApplicationHelper helper(this, QStringLiteral("activeFocusOnClose1.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickPopup *focusedPopup = helper.appWindow->property("focusedPopup").value<QQuickPopup*>();
    QVERIFY(focusedPopup);

    QQuickPopup *nonFocusedPopup = helper.appWindow->property("nonFocusedPopup").value<QQuickPopup*>();
    QVERIFY(nonFocusedPopup);

    focusedPopup->open();
    QVERIFY(focusedPopup->isVisible());
    QTRY_VERIFY(focusedPopup->isOpened());
    QVERIFY(focusedPopup->hasActiveFocus());

    nonFocusedPopup->open();
    QVERIFY(nonFocusedPopup->isVisible());
    QTRY_VERIFY(nonFocusedPopup->isOpened());
    QVERIFY(focusedPopup->hasActiveFocus());

    nonFocusedPopup->close();
    QTRY_VERIFY(!nonFocusedPopup->isVisible());
    QVERIFY(focusedPopup->hasActiveFocus());

    // QTBUG-66113: force active focus on a popup that did not request focus
    nonFocusedPopup->open();
    nonFocusedPopup->forceActiveFocus();
    QVERIFY(nonFocusedPopup->isVisible());
    QTRY_VERIFY(nonFocusedPopup->isOpened());
    QVERIFY(nonFocusedPopup->hasActiveFocus());

    nonFocusedPopup->close();
    QTRY_VERIFY(!nonFocusedPopup->isVisible());
    QVERIFY(focusedPopup->hasActiveFocus());
}

void tst_QQuickPopup::activeFocusOnClose2()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    // Test that a popup that sets focus: true but relinquishes focus (e.g. by
    // calling forceActiveFocus() on another item) before it closes doesn't
    // affect the active focus item when it closes.
    QQuickControlsApplicationHelper helper(this, QStringLiteral("activeFocusOnClose2.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickPopup *popup1 = helper.appWindow->property("popup1").value<QQuickPopup*>();
    QVERIFY(popup1);

    QQuickPopup *popup2 = helper.appWindow->property("popup2").value<QQuickPopup*>();
    QVERIFY(popup2);

    QQuickButton *closePopup2Button = helper.appWindow->property("closePopup2Button").value<QQuickButton*>();
    QVERIFY(closePopup2Button);

    popup1->open();
    QVERIFY(popup1->isVisible());
    QTRY_VERIFY(popup1->isOpened());
    QVERIFY(popup1->hasActiveFocus());

    popup2->open();
    QVERIFY(popup2->isVisible());
    QTRY_VERIFY(popup2->isOpened());
    QVERIFY(popup2->hasActiveFocus());

    // Causes popup1.contentItem.forceActiveFocus() to be called, then closes popup2.
    QTRY_VERIFY(closePopup2Button->width() > 0);
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
        closePopup2Button->mapToScene(QPointF(closePopup2Button->width() / 2, closePopup2Button->height() / 2)).toPoint());
    QTRY_VERIFY(!popup2->isVisible());
    QVERIFY(popup1->hasActiveFocus());
}

void tst_QQuickPopup::activeFocusOnClose3()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    // Test that a closing popup that had focus doesn't steal focus from
    // another popup that the focus was transferred to.
    QQuickControlsApplicationHelper helper(this, QStringLiteral("activeFocusOnClose3.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickPopup *popup1 = helper.appWindow->property("popup1").value<QQuickPopup*>();
    QVERIFY(popup1);

    QQuickPopup *popup2 = helper.appWindow->property("popup2").value<QQuickPopup*>();
    QVERIFY(popup2);

    popup1->open();
    QVERIFY(popup1->isVisible());
    QTRY_VERIFY(popup1->hasActiveFocus());

    popup2->open();
    popup1->close();

    QSignalSpy closedSpy(popup1, SIGNAL(closed()));
    QVERIFY(closedSpy.isValid());
    QVERIFY(closedSpy.wait());

    QVERIFY(!popup1->isVisible());
    QTRY_VERIFY(popup2->isVisible());
    QTRY_VERIFY(popup2->hasActiveFocus());
}

void tst_QQuickPopup::activeFocusOnClosingSeveralPopups()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    // Test that active focus isn't lost when multiple popup closing simultaneously
    QQuickControlsApplicationHelper helper(this, QStringLiteral("activeFocusOnClosingSeveralPopups.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickItem *button = window->property("button").value<QQuickItem *>();
    QVERIFY(button);

    QQuickPopup *popup1 = window->property("popup1").value<QQuickPopup *>();
    QVERIFY(popup1);

    QQuickPopup *popup2 = window->property("popup2").value<QQuickPopup *>();
    QVERIFY(popup2);

    QCOMPARE(button->hasActiveFocus(), true);
    popup1->open();
    QTRY_VERIFY(popup1->isOpened());
    QVERIFY(popup1->hasActiveFocus());
    popup2->open();
    QTRY_VERIFY(popup2->isOpened());
    QVERIFY(popup2->hasActiveFocus());
    QTRY_COMPARE(button->hasActiveFocus(), false);
    // close the unfocused popup first
    popup1->close();
    popup2->close();
    QTRY_VERIFY(!popup1->isVisible());
    QTRY_VERIFY(!popup2->isVisible());
    QTRY_COMPARE(button->hasActiveFocus(), true);

    popup1->open();
    QTRY_VERIFY(popup1->isOpened());
    QVERIFY(popup1->hasActiveFocus());
    popup2->open();
    QTRY_VERIFY(popup2->isOpened());
    QVERIFY(popup2->hasActiveFocus());
    QTRY_COMPARE(button->hasActiveFocus(), false);
    // close the focused popup first
    popup2->close();
    popup1->close();
    QTRY_VERIFY(!popup1->isVisible());
    QTRY_VERIFY(!popup2->isVisible());
    QTRY_COMPARE(button->hasActiveFocus(), true);
}

void tst_QQuickPopup::activeFocusAfterExit()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    // Test that after closing a popup the highest one in z-order receives it instead.
    QQuickControlsApplicationHelper helper(this, QStringLiteral("activeFocusAfterExit.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickPopup *popup1 = window->property("popup1").value<QQuickPopup*>();
    QVERIFY(popup1);

    QQuickPopup *popup2 = window->property("popup2").value<QQuickPopup*>();
    QVERIFY(popup2);
    QSignalSpy closedSpy2(popup2, SIGNAL(closed()));
    QVERIFY(closedSpy2.isValid());

    QQuickPopup *popup3 = window->property("popup3").value<QQuickPopup*>();
    QVERIFY(popup3);
    QSignalSpy closedSpy3(popup3, SIGNAL(closed()));
    QVERIFY(closedSpy3.isValid());

    popup1->open();
    QVERIFY(popup1->isVisible());
    QTRY_VERIFY(popup1->hasActiveFocus());

    popup2->open();
    QVERIFY(popup2->isVisible());
    QTRY_VERIFY(!popup2->hasActiveFocus());

    popup3->open();
    QVERIFY(popup3->isVisible());
    QTRY_VERIFY(popup3->hasActiveFocus());

    popup3->close();
    closedSpy3.wait();
    QVERIFY(!popup3->isVisible());
    QTRY_VERIFY(!popup3->hasActiveFocus());
    QTRY_VERIFY(!popup2->hasActiveFocus());
    QTRY_VERIFY(popup1->hasActiveFocus());

    popup2->close();
    closedSpy2.wait();
    QVERIFY(!popup2->isVisible());
    QTRY_VERIFY(!popup2->hasActiveFocus());
    QTRY_VERIFY(popup1->hasActiveFocus());
}

void tst_QQuickPopup::activeFocusOnDelayedEnter()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    // Test that after opening two popups, first of which has an animation, does not cause
    // the first one to receive focus after the animation stops.
    QQuickControlsApplicationHelper helper(this, QStringLiteral("activeFocusOnDelayedEnter.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickPopup *popup1 = window->property("popup1").value<QQuickPopup*>();
    QVERIFY(popup1);
    QSignalSpy openedSpy(popup1, SIGNAL(opened()));

    QQuickPopup *popup2 = window->property("popup2").value<QQuickPopup*>();
    QVERIFY(popup2);

    popup1->open();
    popup2->open();
    openedSpy.wait();
    QTRY_VERIFY(popup2->hasActiveFocus());
}

// Test that a popup (popup1) with a lower stacking order than another popup (popup2) gets
// key events due to having active focus.
void tst_QQuickPopup::activeFocusDespiteLowerStackingOrder()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    QQuickControlsApplicationHelper helper(this, QStringLiteral("activeFocusOnClose3.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickPopup *popup1 = window->property("popup1").value<QQuickPopup *>();
    QVERIFY(popup1);
    popup1->setPopupType(QQuickPopup::Item);
    popup1->open();
    QTRY_VERIFY(popup1->isOpened());

    QQuickPopup *popup2 = window->property("popup2").value<QQuickPopup *>();
    QVERIFY(popup2);
    popup2->setPopupType(QQuickPopup::Item);
    popup2->open();
    QTRY_VERIFY(popup2->isOpened());
    popup2->setX(popup1->width() / 2);
    popup2->setY(popup1->height() / 2);

    // Both popups have no explicitly assigned Z value, so they should be the same.
    // Items (QQuickPopupItem in this case) with identical Z values are rendered according
    // to their order in the childItems container in the parent QQuickItem (see
    // paintOrderChildItems(), which is what stackingOrderPopups() uses).
    QCOMPARE(popup1->z(), popup2->z());

    // Give popup1 active focus. Even though it's stacked under popup2,
    // it should still receive key events.
    popup1->forceActiveFocus();

    // Press Escape to close popup1.
    QTest::keyClick(window, Qt::Key_Escape);
    QVERIFY(!popup1->isOpened());
    QVERIFY(popup2->isOpened());
    QTRY_VERIFY(!popup1->isVisible());
    QVERIFY(!popup1->hasActiveFocus());
}

void tst_QQuickPopup::activeFocusItemAfterWindowInactive()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    QQuickControlsApplicationHelper helper(this, QStringLiteral("activeFocusAfterWindowInactive.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowFocused(window));

    QQuickPopup *popup = helper.appWindow->property("popup").value<QQuickPopup*>();
    QQuickButton *button = helper.appWindow->property("button").value<QQuickButton*>();
    QVERIFY(popup);
    QVERIFY(button);

    popup->open();
    QTRY_VERIFY(popup->isOpened());
    QVERIFY(popup->hasActiveFocus());
    QVERIFY(!button->hasActiveFocus());

    popup->close();
    QTRY_VERIFY(!popup->isVisible());
    QVERIFY(button->hasActiveFocus());
    QCOMPARE(window->activeFocusItem(), button);

    popup->open();
    QVERIFY(popup->isVisible());
    QTRY_VERIFY(popup->isOpened());

    QQuickWindow newWindow;
    newWindow.setTitle("newFocusWindow");
    newWindow.show();
    newWindow.requestActivate();
    QVERIFY(QTest::qWaitForWindowFocused(&newWindow));

    popup->close();
    QTRY_VERIFY(!popup->isVisible());
    QCOMPARE(QGuiApplication::focusWindow(), &newWindow);

    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowFocused(window));
    QTRY_COMPARE(window->activeFocusItem(), button);
}

void tst_QQuickPopup::hover_data()
{
    QTest::addColumn<QString>("source");
    QTest::addColumn<bool>("modal");

    QTest::newRow("Window:modal") << "window-hover.qml" << true;
    QTest::newRow("Window:modeless") << "window-hover.qml" << false;
    QTest::newRow("ApplicationWindow:modal") << "applicationwindow-hover.qml" << true;
    QTest::newRow("ApplicationWindow:modeless") << "applicationwindow-hover.qml" << false;
}

void tst_QQuickPopup::hover()
{
    QFETCH(QString, source);
    QFETCH(bool, modal);

    QQuickControlsApplicationHelper helper(this, source);
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *popup = window->property("popup").value<QQuickPopup*>();
    QVERIFY(popup);
    popup->setModal(modal);

    QQuickButton *parentButton = window->property("parentButton").value<QQuickButton*>();
    QVERIFY(parentButton);
    parentButton->setHoverEnabled(true);

    QQuickButton *childButton = window->property("childButton").value<QQuickButton*>();
    QVERIFY(childButton);
    childButton->setHoverEnabled(true);

    QSignalSpy openedSpy(popup, SIGNAL(opened()));
    QVERIFY(openedSpy.isValid());
    popup->open();
    QVERIFY(openedSpy.size() == 1 || openedSpy.wait());
    QTRY_VERIFY(popup->width() > 10); // somehow this can take a short time with macOS style

    // Hover the parent button outside the popup. It has 10 pixel anchor margins around the window.
    PointLerper pointLerper(window);
    pointLerper.move(15, 15);
    QCOMPARE(parentButton->isHovered(), !modal);
    QVERIFY(!childButton->isHovered());

    // Hover the popup background. Its top-left is 10 pixels in from its parent.
    pointLerper.move(25, 25);
    QVERIFY(!parentButton->isHovered());
    QVERIFY(!childButton->isHovered());

    // Hover the child button in a popup.
    pointLerper.move(mapCenterToWindow(childButton));
    QVERIFY(!parentButton->isHovered());
    QVERIFY(childButton->isHovered());

    QSignalSpy closedSpy(popup, SIGNAL(closed()));
    QVERIFY(closedSpy.isValid());
    popup->close();
    QVERIFY(closedSpy.size() == 1 || closedSpy.wait());

    // hover the parent button after closing the popup
    QTest::mouseMove(window, QPoint(window->width() / 2, window->height() / 2));
    QVERIFY(parentButton->isHovered());
}

#if QT_CONFIG(wheelevent)
void tst_QQuickPopup::wheel_data()
{
    QTest::addColumn<QString>("source");
    QTest::addColumn<bool>("modal");

    QTest::newRow("Window:modal") << "window-wheel.qml" << true;
    QTest::newRow("Window:modeless") << "window-wheel.qml" << false;
    QTest::newRow("ApplicationWindow:modal") << "applicationwindow-wheel.qml" << true;
    QTest::newRow("ApplicationWindow:modeless") << "applicationwindow-wheel.qml" << false;
}

static bool sendWheelEvent(QQuickItem *item, const QPointF &localPos, int degrees)
{
    QQuickWindow *window = item->window();
    const QPoint scenePos = item->mapToScene(localPos).toPoint();
    QWheelEvent wheelEvent(scenePos, window->mapToGlobal(scenePos), QPoint(0, 0),
                           QPoint(0, 8 * degrees), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase,
                           false);
    QSpontaneKeyEvent::setSpontaneous(&wheelEvent);
    return qGuiApp->notify(window, &wheelEvent);
}

static bool sendWheelEvent(QQuickItem *item, int degrees)
{
    const QPointF localPos = QPointF(item->width() / 2, item->height() / 2);
    return sendWheelEvent(item, localPos, degrees);
}

void tst_QQuickPopup::wheel()
{
    QFETCH(QString, source);
    QFETCH(bool, modal);

    QQuickControlsApplicationHelper helper(this, source);
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickSlider *contentSlider = window->property("contentSlider").value<QQuickSlider*>();
    QVERIFY(contentSlider);

    QQuickPopup *popup = window->property("popup").value<QQuickPopup*>();
    QVERIFY(popup && popup->contentItem());
    popup->setModal(modal);

    QQuickPopup *nestedPopup = window->property("nestedPopup").value<QQuickPopup*>();
    QVERIFY(nestedPopup && nestedPopup->contentItem());
    nestedPopup->setModal(modal);

    QQuickSlider *popupSlider = window->property("popupSlider").value<QQuickSlider*>();
    QVERIFY(popupSlider);

    {
        // wheel over the content
        qreal oldContentValue = contentSlider->value();
        qreal oldPopupValue = popupSlider->value();

        QVERIFY(sendWheelEvent(contentSlider, 15));

        QVERIFY(!qFuzzyCompare(contentSlider->value(), oldContentValue)); // must have moved
        QVERIFY(qFuzzyCompare(popupSlider->value(), oldPopupValue)); // must not have moved
    }

    QSignalSpy openedSpy(popup, SIGNAL(opened()));
    QVERIFY(openedSpy.isValid());
    popup->open();
    QVERIFY(openedSpy.size() == 1 || openedSpy.wait());

    {
        // wheel over the popup content
        qreal oldContentValue = contentSlider->value();
        qreal oldPopupValue = popupSlider->value();

        QVERIFY(sendWheelEvent(popupSlider, 15));

        QVERIFY(qFuzzyCompare(contentSlider->value(), oldContentValue)); // must not have moved
        QVERIFY(!qFuzzyCompare(popupSlider->value(), oldPopupValue)); // must have moved
    }

    QSignalSpy nestedOpenedSpy(nestedPopup, SIGNAL(opened()));
    QVERIFY(nestedOpenedSpy.isValid());
    nestedPopup->open();
    QVERIFY(nestedOpenedSpy.size() == 1 || nestedOpenedSpy.wait());

    {
        // wheel over the popup content
        qreal oldContentValue = contentSlider->value();
        qreal oldPopupValue = popupSlider->value();

        QVERIFY(sendWheelEvent(popupSlider, 15));

        QVERIFY(qFuzzyCompare(contentSlider->value(), oldContentValue)); // must not have moved
        QCOMPARE(qFuzzyCompare(popupSlider->value(), oldPopupValue), modal); // must not have moved unless modeless
    }

    {
        // wheel over the overlay
        qreal oldContentValue = contentSlider->value();
        qreal oldPopupValue = popupSlider->value();

        QVERIFY(sendWheelEvent(QQuickOverlay::overlay(window), QPointF(0, 0), 15));

        if (modal) {
            // the content below a modal overlay must not move
            QVERIFY(qFuzzyCompare(contentSlider->value(), oldContentValue));
        } else {
            // the content below a modeless overlay must move
            QVERIFY(!qFuzzyCompare(contentSlider->value(), oldContentValue));
        }
        QVERIFY(qFuzzyCompare(popupSlider->value(), oldPopupValue)); // must not have moved
    }
}
#endif

void tst_QQuickPopup::parentDestroyed()
{
    QQuickPopup popup;
    popup.setParentItem(new QQuickItem);
    delete popup.parentItem();
    QVERIFY(!popup.parentItem());
}

void tst_QQuickPopup::nested()
{
    QQuickControlsApplicationHelper helper(this, QStringLiteral("nested.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *modalPopup = window->property("modalPopup").value<QQuickPopup *>();
    QVERIFY(modalPopup);

    QQuickPopup *modelessPopup = window->property("modelessPopup").value<QQuickPopup *>();
    QVERIFY(modelessPopup);

    modalPopup->open();
    QCOMPARE(modalPopup->isVisible(), true);
    QTRY_COMPARE(modalPopup->isOpened(), true);

    modelessPopup->open();
    QCOMPARE(modelessPopup->isVisible(), true);
    QTRY_COMPARE(modelessPopup->isOpened(), true);

    // click outside the modeless popup on the top, but inside the modal popup below
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(150, 150));

    QTRY_COMPARE(modelessPopup->isVisible(), false);
    QCOMPARE(modalPopup->isVisible(), true);
}

#if QT_CONFIG(wheelevent)
void tst_QQuickPopup::nestedWheel()
{
    QQuickControlsApplicationHelper helper(this, QStringLiteral("nested-wheel.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *modalPopup = window->property("modalPopup").value<QQuickPopup *>();
    QVERIFY(modalPopup);

    QQuickComboBox *comboBox = window->property("comboBox").value<QQuickComboBox *>();
    QVERIFY(comboBox);

    const QPoint comboBoxCenter = comboBox->mapToScene(
        QPointF(comboBox->width() / 2, comboBox->height() / 2)).toPoint();
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, comboBoxCenter);
    QTRY_VERIFY(comboBox->popup()->isOpened());

    QQuickItem *listView = comboBox->popup()->contentItem();
    QVERIFY(listView);
    QQuickItem *vbar = listView->findChild<QQuickItem *>("vbar");
    QVERIFY(vbar);

    const double startPosition = vbar->property("position").toDouble();
    // wheel over the list view, verify that it scrolls
    sendWheelEvent(listView, -30);
    QTRY_COMPARE_GT(vbar->property("position").toDouble(), startPosition);
}

void tst_QQuickPopup::nestedWheelWithOverlayParent()
{
    QQuickControlsApplicationHelper helper(this, QStringLiteral("nested-wheel-overlay-parent.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *drawer= window->property("_drawer").value<QQuickDrawer *>();
    QVERIFY(drawer);

    auto *dropArea = window->property("_dropArea").value<QQuickDropArea *>();
    QVERIFY(dropArea);

    drawer->open();
    QCOMPARE(drawer->isVisible(), true);
    QTRY_COMPARE(drawer->isOpened(), true);

    QQuickListView *listView = window->property("_listView").value<QQuickListView *>();
    QTRY_VERIFY(listView != nullptr);
    QQuickItem *contentItem = listView->contentItem();
    QTRY_VERIFY(contentItem != nullptr);

    // Check parent is set as overlay
    QTRY_COMPARE(dropArea->parentItem(), QQuickOverlay::overlay(window));
    // Consider the center point of the control as event position to trigger wheel event
    QVERIFY(sendWheelEvent(listView, -15));

    if (QQuickTest::qIsPolishScheduled(listView))
        QVERIFY(QQuickTest::qWaitForPolish(listView));

    // Wheel over the list view, verify that it scrolls
    QTRY_COMPARE(listView->contentY(), 72.);
}
#endif

void tst_QQuickPopup::modelessOnModalOnModeless()
{
    QQuickControlsApplicationHelper helper(this, QStringLiteral("modelessOnModalOnModeless.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *modelessPopup = window->property("modelessPopup").value<QQuickPopup *>();
    QVERIFY(modelessPopup);

    QQuickButton *button = window->property("button").value<QQuickButton *>();
    QVERIFY(button);
    QQuickPopup *modalPopup = window->property("modalPopup").value<QQuickPopup *>();
    QVERIFY(modalPopup);
    QQuickPopup *tooltip = window->property("tooltip").value<QQuickPopup *>();
    QVERIFY(tooltip);

    modelessPopup->open();
    QCOMPARE(modelessPopup->isVisible(), true);
    QTRY_COMPARE(modelessPopup->isOpened(), true);

    const auto buttonPoint = button->mapToScene(button->boundingRect().center()).toPoint();
    // click into the button, should not be blocked
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, buttonPoint);
    QVERIFY(button->isChecked());

    modalPopup->open();
    QCOMPARE(modalPopup->isVisible(), true);
    QTRY_COMPARE(modalPopup->isOpened(), true);
    // click into the button, should be blocked
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, buttonPoint);
    QVERIFY(button->isChecked());

    tooltip->setVisible(true);
    QCOMPARE(tooltip->isVisible(), true);
    QTRY_COMPARE(tooltip->isOpened(), true);
    // click into the button, should be blocked
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, buttonPoint);
    QVERIFY(button->isChecked());
}

// QTBUG-56697
void tst_QQuickPopup::grabber()
{
    QQuickControlsApplicationHelper helper(this, QStringLiteral("grabber.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *menu = window->property("menu").value<QQuickPopup *>();
    QVERIFY(menu);

    QQuickPopup *popup = window->property("popup").value<QQuickPopup *>();
    QVERIFY(popup);

    QQuickPopup *combo = window->property("combo").value<QQuickPopup *>();
    QVERIFY(combo);

    menu->open();
    QTRY_COMPARE(menu->isOpened(), true);
    QCOMPARE(popup->isVisible(), false);
    QCOMPARE(combo->isVisible(), false);

    // click a menu item to open the popup
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(menu->x() + menu->width() / 2, menu->y() + menu->height() / 2));
    QTRY_COMPARE(menu->isVisible(), false);
    QTRY_COMPARE(popup->isOpened(), true);
    QCOMPARE(combo->isVisible(), false);

    combo->open();
    QCOMPARE(menu->isVisible(), false);
    QCOMPARE(popup->isVisible(), true);
    QTRY_COMPARE(combo->isOpened(), true);

    // click outside to close both the combo popup and the parent popup
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(window->width() - 1, window->height() - 1));
    QCOMPARE(menu->isVisible(), false);
    QTRY_COMPARE(popup->isVisible(), false);
    QTRY_COMPARE(combo->isVisible(), false);

    menu->open();
    QTRY_COMPARE(menu->isOpened(), true);
    QCOMPARE(popup->isVisible(), false);
    QCOMPARE(combo->isVisible(), false);

    // click outside the menu to close it (QTBUG-56697)
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(window->width() - 1, window->height() - 1));
    QTRY_COMPARE(menu->isVisible(), false);
    QCOMPARE(popup->isVisible(), false);
    QCOMPARE(combo->isVisible(), false);
}

void tst_QQuickPopup::cursorShape()
{
    // Ensure that the mouse cursor has the correct shape when over a popup
    // which is itself over an item with a different shape.
    QQuickControlsApplicationHelper helper(this, QStringLiteral("cursor.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *popup = helper.appWindow->property("popup").value<QQuickPopup*>();
    QVERIFY(popup);

    popup->open();
    QVERIFY(popup->isVisible());
    QTRY_VERIFY(popup->isOpened());

    QQuickItem *textField = helper.appWindow->property("textField").value<QQuickItem*>();
    QVERIFY(textField);

    // Move the mouse over the text field.
    const QPoint textFieldPos(popup->x() - 1, textField->height() / 2);
    QVERIFY(textField->contains(textField->mapFromScene(textFieldPos)));
    QTest::mouseMove(window, textFieldPos);
    QTRY_COMPARE(window->cursor().shape(), textField->cursor().shape());

    // Move the mouse over the popup where it overlaps with the text field.
    const QPoint textFieldOverlapPos(popup->x() + 1, textField->height() / 2);
    QTest::mouseMove(window, textFieldOverlapPos);
    QTRY_COMPARE(window->cursor().shape(), popup->popupItem()->cursor().shape());

    popup->close();
    QTRY_VERIFY(!popup->isVisible());
}

class FriendlyPopup : public QQuickPopup
{
    friend class tst_QQuickPopup;
};

void tst_QQuickPopup::componentComplete()
{
    FriendlyPopup cppPopup;
    QVERIFY(cppPopup.isComponentComplete());

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import QtQuick.Controls; Popup { }", QUrl());

    QScopedPointer<QObject> o(component.beginCreate(engine.rootContext()));
    FriendlyPopup *qmlPopup = static_cast<FriendlyPopup *>(o.data());
    QVERIFY(qmlPopup);
    QVERIFY(!qmlPopup->isComponentComplete());

    component.completeCreate();
    QVERIFY(qmlPopup->isComponentComplete());
}

void tst_QQuickPopup::closeOnEscapeWithNestedPopups()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    // Tests the scenario in the Gallery example, where there are nested popups that should
    // close in the correct order when the Escape key is pressed.
    QQuickControlsApplicationHelper helper(this, QStringLiteral("closeOnEscapeWithNestedPopups.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickApplicationWindow *window = helper.appWindow;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    // The stack view should have two items, and it should pop the second when escape is pressed
    // and it has focus.
    QQuickStackView *stackView = window->findChild<QQuickStackView*>("stackView");
    QVERIFY(stackView);
    QCOMPARE(stackView->depth(), 2);

    QQuickItem *optionsToolButton = window->findChild<QQuickItem*>("optionsToolButton");
    QVERIFY(optionsToolButton);

    // Click on the options tool button. The settings menu should pop up.
    const QPoint optionsToolButtonCenter = optionsToolButton->mapToScene(
        QPointF(optionsToolButton->width() / 2, optionsToolButton->height() / 2)).toPoint();
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, optionsToolButtonCenter);

    QQuickPopup *optionsMenu = window->findChild<QQuickPopup*>("optionsMenu");
    QVERIFY(optionsMenu);
    QTRY_VERIFY(optionsMenu->isOpened());

    QQuickItem *settingsMenuItem = window->findChild<QQuickItem*>("settingsMenuItem");
    QVERIFY(settingsMenuItem);

    QQuickPopup *settingsDialog = window->contentItem()->findChild<QQuickPopup*>("settingsDialog");
    QVERIFY(settingsDialog);
    settingsDialog->setPopupType(QQuickPopup::Item);

    // Click on the settings menu item. The settings dialog should pop up.
    const QPoint settingsMenuItemCenter = settingsMenuItem->mapToScene(
        QPointF(settingsMenuItem->width() / 2, settingsMenuItem->height() / 2)).toPoint();
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, settingsMenuItemCenter);

    QTRY_VERIFY(!optionsMenu->isVisible());
    QTRY_VERIFY(settingsDialog->isOpened());

    QQuickComboBox *comboBox = window->contentItem()->findChild<QQuickComboBox*>("comboBox");
    QVERIFY(comboBox);

    // Click on the combo box button. The combo box popup should pop up.
    const QPoint comboBoxCenter = comboBox->mapToScene(
        QPointF(comboBox->width() / 2, comboBox->height() / 2)).toPoint();
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, comboBoxCenter);
    QTRY_VERIFY(comboBox->popup()->isOpened());

    // Close the combo box popup with the escape key. The settings dialog should still be visible.
    QTest::keyClick(window, Qt::Key_Escape);
    QTRY_VERIFY(!comboBox->popup()->isVisible());
    QVERIFY(settingsDialog->isVisible());

    // Close the settings dialog with the escape key.
    QTest::keyClick(window, Qt::Key_Escape);
    QTRY_VERIFY(!settingsDialog->isVisible());

    // The stack view should still have two items.
    QCOMPARE(stackView->depth(), 2);

    // Remove one by pressing the Escape key (the Shortcut should be activated).
    QTest::keyClick(window, Qt::Key_Escape);
    QCOMPARE(stackView->depth(), 1);
}

void tst_QQuickPopup::closeOnEscapeWithVisiblePopup()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    QQuickControlsApplicationHelper helper(this, QStringLiteral("closeOnEscapeWithVisiblePopup.qml"));
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickPopup *popup = window->findChild<QQuickPopup *>("popup");
    QVERIFY(popup);
    QTRY_VERIFY(popup->isOpened());

    QTRY_VERIFY(popup->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Escape);
    QTRY_VERIFY(!popup->isVisible());
}

void tst_QQuickPopup::enabled()
{
    QQuickPopup popup;
    QVERIFY(popup.isEnabled());
    QVERIFY(popup.popupItem()->isEnabled());

    QSignalSpy enabledSpy(&popup, &QQuickPopup::enabledChanged);
    QVERIFY(enabledSpy.isValid());

    popup.setEnabled(false);
    QVERIFY(!popup.isEnabled());
    QVERIFY(!popup.popupItem()->isEnabled());
    QCOMPARE(enabledSpy.size(), 1);

    popup.popupItem()->setEnabled(true);
    QVERIFY(popup.isEnabled());
    QVERIFY(popup.popupItem()->isEnabled());
    QCOMPARE(enabledSpy.size(), 2);
}

void tst_QQuickPopup::orientation_data()
{
    QTest::addColumn<Qt::ScreenOrientation>("orientation");

    QTest::newRow("Portrait") << Qt::PortraitOrientation;
    QTest::newRow("Landscape") << Qt::LandscapeOrientation;
    QTest::newRow("InvertedPortrait") << Qt::InvertedPortraitOrientation;
    QTest::newRow("InvertedLandscape") << Qt::InvertedLandscapeOrientation;
}

/*
    Verify that the popup is centered, and that the dimmer and overlay cover
    the content item for any content orientation and content rotation.
*/
void tst_QQuickPopup::orientation()
{
    QFETCH(Qt::ScreenOrientation, orientation);

    QQuickControlsApplicationHelper helper(this, "orientation.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *popup = window->property("popup").value<QQuickPopup *>();
    QVERIFY(popup);
    QTRY_VERIFY(popup->isOpened());

    QQuickItem *dimmer = QQuickPopupPrivate::get(popup)->dimmer;
    QVERIFY(dimmer);
    QQuickItem *popupItem = popup->popupItem();
    QVERIFY(popupItem);
    QQuickOverlay *overlay = QQuickOverlay::overlay(window);
    QVERIFY(overlay);
    QQuickItem *contentItem = window->contentItem();
    QVERIFY(contentItem);

    const auto rotation = window->screen()->angleBetween(Qt::PrimaryOrientation, orientation);
    QCOMPARE(popupItem->mapToScene(popupItem->boundingRect().center()), contentItem->boundingRect().center());
    QCOMPARE(dimmer->boundingRect(), contentItem->boundingRect());
    QCOMPARE(overlay->boundingRect(), contentItem->boundingRect());
    window->contentItem()->setRotation(rotation);
    QCOMPARE(popupItem->mapToScene(popupItem->boundingRect().center()), contentItem->boundingRect().center());
    QCOMPARE(dimmer->boundingRect(), contentItem->boundingRect());
    QCOMPARE(overlay->boundingRect(), contentItem->boundingRect());
}

void tst_QQuickPopup::qquickview()
{
    QQuickView view;
    view.setObjectName("QQuickView");
    view.resize(400, 400);
    view.setSource(testFileUrl("dialog.qml"));
    QVERIFY(view.status() != QQuickView::Error);
    view.contentItem()->setObjectName("QQuickViewContentItem");
    view.show();

    QQuickDialog *dialog = view.rootObject()->property("dialog").value<QQuickDialog*>();
    QVERIFY(dialog);
    QTRY_COMPARE(dialog->property("opened").toBool(), true);

    dialog->close();
    QTRY_COMPARE(dialog->property("visible").toBool(), false);

    // QTBUG-72746: shouldn't crash on application exit after closing a Dialog when using QQuickView.
}

// TODO: also test it out without setting enabled directly on menu, but on a parent

// QTBUG-73447
void tst_QQuickPopup::disabledPalette()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    QQuickControlsApplicationHelper helper(this, "disabledPalette.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickPopup *popup = window->property("popup").value<QQuickPopup*>();
    QVERIFY(popup);

    QSignalSpy popupEnabledSpy(popup, &QQuickPopup::enabledChanged);
    QVERIFY(popupEnabledSpy.isValid());
    QSignalSpy popupPaletteSpy(popup, &QQuickPopup::paletteChanged);
    QVERIFY(popupPaletteSpy.isValid());

    QSignalSpy popupItemEnabledSpy(popup->popupItem(), &QQuickItem::enabledChanged);
    QVERIFY(popupItemEnabledSpy.isValid());
    QSignalSpy popupItemPaletteSpy(popup->popupItem(), &QQuickItem::paletteChanged);
    QVERIFY(popupItemPaletteSpy.isValid());

    auto palette = QQuickPopupPrivate::get(popup)->palette();
    palette->setBase(Qt::green);
    palette->disabled()->setBase(Qt::red);
    QCOMPARE(popupPaletteSpy.size(), 2);
    QCOMPARE(popupItemPaletteSpy.size(), 2);
    QCOMPARE(popup->background()->property("color").value<QColor>(), Qt::green);

    popup->setEnabled(false);
    QCOMPARE(popupEnabledSpy.size(), 1);
    QCOMPARE(popupItemEnabledSpy.size(), 1);
    QCOMPARE(popupPaletteSpy.size(), 3);
    QCOMPARE(popupItemPaletteSpy.size(), 3);
    QCOMPARE(popup->background()->property("color").value<QColor>(), Qt::red);
}

void tst_QQuickPopup::disabledParentPalette()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    QQuickControlsApplicationHelper helper(this, "disabledPalette.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickPopup *popup = window->property("popup").value<QQuickPopup*>();
    QVERIFY(popup);

    QSignalSpy popupEnabledSpy(popup, SIGNAL(enabledChanged()));
    QVERIFY(popupEnabledSpy.isValid());
    QSignalSpy popupPaletteSpy(popup, SIGNAL(paletteChanged()));
    QVERIFY(popupPaletteSpy.isValid());

    QSignalSpy popupItemEnabledSpy(popup->popupItem(), SIGNAL(enabledChanged()));
    QVERIFY(popupItemEnabledSpy.isValid());
    QSignalSpy popupItemPaletteSpy(popup->popupItem(), SIGNAL(paletteChanged()));
    QVERIFY(popupItemPaletteSpy.isValid());

    auto palette = QQuickPopupPrivate::get(popup)->palette();
    palette->setBase(Qt::green);
    palette->disabled()->setBase(Qt::red);
    QCOMPARE(popupPaletteSpy.size(), 2);
    QCOMPARE(popupItemPaletteSpy.size(), 2);
    QCOMPARE(popup->background()->property("color").value<QColor>(), Qt::green);

    // Disable the overlay (which is QQuickPopupItem's parent) to ensure that
    // the palette is changed when the popup is indirectly disabled.
    popup->open();
    QTRY_VERIFY(popup->isOpened());
    QVERIFY(QMetaObject::invokeMethod(window, "disableOverlay"));
    QVERIFY(!popup->isEnabled());
    QVERIFY(!popup->popupItem()->isEnabled());
    QCOMPARE(popup->background()->property("color").value<QColor>(), Qt::red);
    QCOMPARE(popupEnabledSpy.size(), 1);
    QCOMPARE(popupItemEnabledSpy.size(), 1);
    QCOMPARE(popupPaletteSpy.size(), 3);
    QCOMPARE(popupItemPaletteSpy.size(), 3);

    popup->close();
    QTRY_VERIFY(!popup->isVisible());
}

void tst_QQuickPopup::countChanged()
{
    QQuickControlsApplicationHelper helper(this, "countChanged.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickComboBox *comboBox = window->property("comboBox").value<QQuickComboBox*>();
    QVERIFY(comboBox);
    QCOMPARE(window->property("count").toInt(), 1);

    QVERIFY(window->setProperty("isModel1", false));
    QTRY_COMPARE(window->property("count").toInt(), 2);
}

// QTBUG-73243
void tst_QQuickPopup::toolTipCrashOnClose()
{
    if (!canImportModule("import QtGraphicalEffects; DropShadow {}"))
        QSKIP("Test requires QtGraphicalEffects");

    QQuickControlsApplicationHelper helper(this, "toolTipCrashOnClose.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QTest::mouseMove(window, QPoint(window->width() / 2, window->height() / 2));
    QTRY_VERIFY(window->property("toolTipOpened").toBool());

    QVERIFY(window->close());
    // Shouldn't crash.
}

void tst_QQuickPopup::setOverlayParentToNull()
{
    if (!canImportModule("import QtGraphicalEffects; DropShadow {}"))
        QSKIP("Test requires QtGraphicalEffects");

    QQuickControlsApplicationHelper helper(this, "toolTipCrashOnClose.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QVERIFY(QMetaObject::invokeMethod(window, "nullifyOverlayParent"));

    QTest::mouseMove(window, QPoint(window->width() / 2, window->height() / 2));
    QTRY_VERIFY(window->property("toolTipOpened").toBool());

    QVERIFY(window->close());
    // While nullifying the overlay parent doesn't make much sense, it shouldn't crash.
}

void tst_QQuickPopup::tabFence()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    if (QGuiApplication::styleHints()->tabFocusBehavior() != Qt::TabFocusAllControls)
        QSKIP("This platform only allows tab focus for text controls");

    QQuickControlsApplicationHelper helper(this, "tabFence.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    QQuickPopup *popup = window->property("dialog").value<QQuickPopup*>();
    QVERIFY(popup);
    popup->open();
    QTRY_VERIFY(popup->isOpened());

    QQuickButton *outsideButton1 = window->property("outsideButton1").value<QQuickButton*>();
    QVERIFY(outsideButton1);
    QQuickButton *outsideButton2 = window->property("outsideButton2").value<QQuickButton*>();
    QVERIFY(outsideButton2);
    QQuickButton *dialogButton1 = window->property("dialogButton1").value<QQuickButton*>();
    QVERIFY(dialogButton1);
    QQuickButton *dialogButton2 = window->property("dialogButton2").value<QQuickButton*>();
    QVERIFY(dialogButton2);

    outsideButton1->forceActiveFocus();
    QVERIFY(outsideButton1->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(outsideButton2->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);

    // Individual styles may set modal: true
    QVERIFY((popup->isModal() ? outsideButton1 : dialogButton1)->QQuickItem::hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY((popup->isModal() ? outsideButton2 : dialogButton2)->QQuickItem::hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(outsideButton1->hasActiveFocus());

    popup->setModal(true);

    // When modal, focus loops between the two external buttons
    outsideButton1->forceActiveFocus();
    QVERIFY(outsideButton1->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(outsideButton2->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(outsideButton1->hasActiveFocus());

    // Same thing for dialog's buttons
    dialogButton1->forceActiveFocus();
    QVERIFY(dialogButton1->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(dialogButton2->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(dialogButton1->hasActiveFocus());

    popup->setModal(false);

    // When not modal, focus goes in and out of the dialog
    outsideButton1->forceActiveFocus();
    QVERIFY(outsideButton1->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(outsideButton2->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(dialogButton1->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(dialogButton2->hasActiveFocus());
    QTest::keyClick(window, Qt::Key_Tab);
    QVERIFY(outsideButton1->hasActiveFocus());
}

void tst_QQuickPopup::invisibleToolTipOpen()
{
    QQuickControlsApplicationHelper helper(this, "invisibleToolTipOpen.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    centerOnScreen(window);
    moveMouseAway(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickItem *mouseArea = qvariant_cast<QQuickItem *>(window->property("mouseArea"));
    QVERIFY(mouseArea);
    auto toolTipAttached = qobject_cast<QQuickToolTipAttached*>(
        qmlAttachedPropertiesObject<QQuickToolTip>(mouseArea, false));
    QVERIFY(toolTipAttached);
    QQuickPopup *toolTip = toolTipAttached->toolTip();
    QVERIFY(toolTip);
    QObject *loader = qvariant_cast<QObject *>(window->property("loader"));
    QVERIFY(loader);

    // Send an extra move event, otherwise the test fails on subsequent runs for different styles for some reason...
    // As an added bonus, this is also slightly more realistic. :D
    QTest::mouseMove(window, QPoint(mouseArea->width() / 2 - 1, mouseArea->height() / 2 - 1));
    QTest::mouseMove(window, QPoint(mouseArea->width() / 2, mouseArea->height() / 2));
    QTRY_VERIFY(toolTip->isOpened());

    QSignalSpy componentLoadedSpy(loader, SIGNAL(loaded()));
    QVERIFY(componentLoadedSpy.isValid());

    loader->setProperty("active", true);
    QTRY_COMPARE(componentLoadedSpy.size(), 1);

    QTRY_VERIFY(toolTip->isVisible());
}

void tst_QQuickPopup::centerInOverlayWithinStackViewItem()
{
    QQuickControlsApplicationHelper helper(this, "centerInOverlayWithinStackViewItem.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *popup = window->property("popup").value<QQuickPopup*>();
    QVERIFY(popup);
    QTRY_COMPARE(popup->isVisible(), true);

    // Shouldn't crash on exit.
}

void tst_QQuickPopup::destroyDuringExitTransition()
{
    SKIP_IF_NO_WINDOW_ACTIVATION

    QQuickControlsApplicationHelper helper(this, "destroyDuringExitTransition.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));

    {
        QPointer<QQuickPopup> dialog2 = window->property("dialog2").value<QQuickPopup*>();
        QVERIFY(dialog2);
        QTRY_COMPARE(dialog2->isOpened(), true);

        // Close the second dialog, destroying it before its exit transition can finish.
        QTest::keyClick(window, Qt::Key_Escape);
        QTRY_VERIFY(!dialog2);
    }

    // Events should go through to the dialog underneath.
    QQuickPopup *dialog1 = window->property("dialog1").value<QQuickPopup*>();
    QVERIFY(dialog1);
    QTRY_COMPARE(dialog1->isOpened(), true);
    QQuickButton *button = dialog1->property("button").value<QQuickButton*>();
    QVERIFY(button);
    const auto buttonClickPos = button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint();
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, buttonClickPos);
    QVERIFY(button->isDown());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, buttonClickPos);
    QVERIFY(!button->isDown());
}

void tst_QQuickPopup::releaseAfterExitTransition()
{
    QQuickApplicationHelper helper(this, "releaseAfterExitTransition.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickOverlay *overlay = QQuickOverlay::overlay(window);
    QQuickPopup *modalPopup = window->property("modalPopup").value<QQuickPopup *>();
    QQuickPopup *popup = window->property("popup").value<QQuickPopup *>();

    modalPopup->open();
    QTRY_VERIFY(modalPopup->isOpened());

    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    // wait until the transition is finished and the overlay hides itself
    QTRY_VERIFY(!overlay->isVisible());
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));

    popup->open();
    QTRY_VERIFY(popup->isOpened());
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QTRY_VERIFY(!popup->isOpened());
}

class ContainmentMask : public QObject
{
    Q_OBJECT
public:
    mutable bool called = false;
    Q_INVOKABLE bool contains(const QPointF &point) const
    {
        called = true;
        // let clicks at {1, 1} through the dimmer
        return point != QPoint(1, 1);
    }
};

/*
    Test case for behavior we rely on in the virtual keyboard:
    To prevent the virtual keyboard from being blocked by modal popups,
    it sets a containment mask on the dimmer item, and lets clicks through
    that hit the virtual keyboard.
*/
void tst_QQuickPopup::dimmerContainmentMask()
{
    ContainmentMask containmentMask;
    int expectedClickCount = 0;

    QQuickApplicationHelper helper(this, "dimmerContainmentMask.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QCOMPARE(window->property("clickCount").toInt(), expectedClickCount);
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickOverlay *overlay = QQuickOverlay::overlay(window);
    QQuickPopup *modalPopup = window->property("modalPopup").value<QQuickPopup *>();
    QVERIFY(modalPopup);

    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QCOMPARE(window->property("clickCount"), ++expectedClickCount);

    modalPopup->open();
    QTRY_VERIFY(modalPopup->isOpened());

    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QCOMPARE(window->property("clickCount"), expectedClickCount); // blocked by modal
    QTRY_VERIFY(!modalPopup->isOpened()); // auto-close

    modalPopup->open();
    QTRY_VERIFY(modalPopup->isOpened());

    QPointer<QQuickItem> dimmer = overlay->property("_q_dimmerItem").value<QQuickItem *>();
    QVERIFY(dimmer);
    dimmer->setContainmentMask(&containmentMask);

    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QVERIFY(containmentMask.called);
    QCOMPARE(window->property("clickCount"), ++expectedClickCount); // let through by containment mask
    QVERIFY(modalPopup->isOpened()); // no auto-close

    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(2, 2));
    QCOMPARE(window->property("clickCount"), expectedClickCount); // blocked by modal
    QTRY_VERIFY(!modalPopup->isOpened()); // auto-close
    QTRY_VERIFY(!dimmer);

    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(1, 1));
    QCOMPARE(window->property("clickCount"), ++expectedClickCount); // no mask left behind
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(2, 2));
    QCOMPARE(window->property("clickCount"), ++expectedClickCount); // no mask left behind
}

void tst_QQuickPopup::shrinkPopupThatWasLargerThanWindow_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow("vertical") << "shrinkPopupThatWasLargerThanWindowHeight.qml";
    QTest::newRow("horizontal") << "shrinkPopupThatWasLargerThanWindowWidth.qml";
}

void tst_QQuickPopup::shrinkPopupThatWasLargerThanWindow()
{
    QFETCH(QString, fileName);

    QQuickApplicationHelper helper(this, fileName);
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *popup = window->property("popup").value<QQuickPopup*>();
    QVERIFY(popup);

    popup->open();
    QTRY_VERIFY(popup->isOpened());

    // Shrink the popup by reducing the model count.
    QVERIFY(window->setProperty("model", 1));

    QVERIFY2(popup->implicitWidth() < window->width(), qPrintable(QString::fromLatin1(
        "Expected popup's implicitWidth (%1) to be less than the window's width (%2)")
            .arg(popup->implicitWidth()).arg(window->width())));
    QVERIFY2(popup->width() < window->width(), qPrintable(QString::fromLatin1(
        "Expected popup's width (%1) to be less than the window's width (%2)")
            .arg(popup->width()).arg(window->width())));

    QVERIFY2(popup->implicitHeight() < window->height(), qPrintable(QString::fromLatin1(
        "Expected popup's implicitHeight (%1) to be less than the window's height (%2)")
            .arg(popup->implicitHeight()).arg(window->height())));
    QVERIFY2(popup->height() < window->height(), qPrintable(QString::fromLatin1(
        "Expected popup's height (%1) to be less than the window's height (%2)")
            .arg(popup->height()).arg(window->height())));
}

void tst_QQuickPopup::relativeZOrder()
{
    QQuickApplicationHelper helper(this, "relativeZOrder.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *parentDialog = window->findChild<QQuickPopup *>("parentDialog");
    QVERIFY(parentDialog);
    auto *subDialog = window->findChild<QQuickPopup *>("subDialog");
    QVERIFY(subDialog);
    auto *parentDialogPrivate = QQuickPopupPrivate::get(parentDialog);
    auto *subDialogPrivate = QQuickPopupPrivate::get(subDialog);

    QVERIFY(!parentDialog->isVisible());
    QVERIFY(!subDialog->isVisible());

    QCOMPARE(parentDialog->popupItem()->parent(), parentDialog);
    QCOMPARE(subDialog->popupItem()->parent(), subDialog);

    parentDialog->open();
    QTRY_VERIFY(parentDialog->isOpened());
    QQuickItem *expectedParentItem = QQuickOverlay::overlay(window);
    if (parentDialogPrivate->usePopupWindow()) {
        QTRY_VERIFY(parentDialogPrivate->popupWindow);
        expectedParentItem = parentDialogPrivate->popupWindow->contentItem();
    }
    QCOMPARE(parentDialog->popupItem()->parentItem(), expectedParentItem);

    subDialog->open();
    QTRY_VERIFY(subDialog->isOpened());
    if (subDialogPrivate->usePopupWindow()) {
        QTRY_VERIFY(subDialogPrivate->popupWindow);
        expectedParentItem = subDialogPrivate->popupWindow->contentItem();
    }
    QCOMPARE(subDialog->popupItem()->parentItem(), expectedParentItem);

    if (!subDialogPrivate->usePopupWindow()) {
        auto *overlayPrivate = QQuickOverlayPrivate::get(QQuickOverlay::overlay(window));
        QCOMPARE(overlayPrivate->paintOrderChildItems().last(), subDialog->popupItem());
    }

    subDialog->close();
    parentDialog->close();
}

void tst_QQuickPopup::mirroredCombobox()
{
#ifdef Q_OS_ANDROID
    // Android screens might be pretty small, such that additional
    // repositioning (apart from the mirroring) will happen to the
    // popups and mess up the expected positions below.
    QSKIP("Skipping test for Android.");
#endif
    QStringList nativeStyles;
    nativeStyles.append(u"macOS"_s);
    nativeStyles.append(u"iOS"_s);
    nativeStyles.append(u"Windows"_s);
    if (nativeStyles.contains(QQuickStyle::name()))
        QSKIP("Skipping test for native styles: they might rearrange their combobox the way they "
              "want.");

    QQuickControlsApplicationHelper helper(this, "mirroredCombobox.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    {
        QQuickComboBox *comboBox = window->findChild<QQuickComboBox *>("first");
        QVERIFY(comboBox);
        QQuickPopup *popup = comboBox->popup();
        QVERIFY(popup);
        popup->setPopupType(QQuickPopup::Item);
        popup->open();
        QTRY_COMPARE(popup->isVisible(), true);
        const QPointF popupPos(popup->contentItem()->mapToItem(comboBox->parentItem(),
                                                               popup->contentItem()->position()));
        const QSizeF popupSize(popup->contentItem()->size());

        // ignore popup.{top,bottom}Padding() as not included in popup->contentItem()->size()
        // some styles prefer to draw the popup "over" (in z-axis direction) the combobox to hide
        // the combobox
        const bool styleDrawsPopupOverCombobox =
                comboBox->position().y() - popupSize.height() + comboBox->size().height()
                == popupPos.y();
        // some styles prefer to draw the popup below (in y-axis direction) the combobox
        const bool styleDrawsPopupBelowCombobox =
                comboBox->position().y() - popupSize.height() + comboBox->topPadding()
                == popupPos.y();

        if (QQuickStyle::name() == QLatin1String("FluentWinUI3"))
            QEXPECT_FAIL("", "Rotated ComboBox is broken in FluentWinUI3 style", Abort);
        QVERIFY(styleDrawsPopupOverCombobox || styleDrawsPopupBelowCombobox);

        popup->close();
    }

    {
        QQuickComboBox *comboBox = window->findChild<QQuickComboBox *>("second");
        QVERIFY(comboBox);
        QQuickPopup *popup = comboBox->popup();
        QVERIFY(popup);
        popup->setPopupType(QQuickPopup::Item);
        popup->open();
        QTRY_COMPARE(popup->isVisible(), true);
        const QPointF popupPos(popup->contentItem()->mapToItem(comboBox->parentItem(),
                                                               popup->contentItem()->position()));

        // some styles prefer to draw the popup "over" (in z-axis direction) the combobox to hide
        // the combobox
        const bool styleDrawsPopupOverCombobox = comboBox->position().y() + comboBox->topPadding()
                        + popup->topPadding() + popup->bottomPadding()
                == popupPos.y();
        // some styles prefer to draw the popup above (in y-axis direction) the combobox
        const bool styleDrawsPopupAboveCombobox =
                comboBox->position().y() + comboBox->height() - comboBox->topPadding()
                == popupPos.y();

        QVERIFY(styleDrawsPopupOverCombobox || styleDrawsPopupAboveCombobox);

        popup->close();
    }
}

void tst_QQuickPopup::rotatedCombobox()
{
#ifdef Q_OS_ANDROID
    // Android screens might be pretty small, such that additional
    // repositioning (apart from the rotating) will happen to the
    // popups and mess up the expected positions below.
    QSKIP("Skipping test for Android.");
#endif
    QStringList nativeStyles;
    nativeStyles.append(u"macOS"_s);
    nativeStyles.append(u"iOS"_s);
    nativeStyles.append(u"Windows"_s);
    if (nativeStyles.contains(QQuickStyle::name()))
        QSKIP("Skipping test for native styles: they might rearrange their combobox the way they "
              "want.");

    QQuickControlsApplicationHelper helper(this, "rotatedCombobox.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    {
        QQuickComboBox *comboBox = window->findChild<QQuickComboBox *>("first");
        QVERIFY(comboBox);
        QQuickPopup *popup = comboBox->popup();
        QVERIFY(popup);
        popup->open();
        QTRY_COMPARE(popup->isVisible(), true);
        const QPointF popupPos(popup->contentItem()->mapToItem(comboBox->parentItem(),
                                                               popup->contentItem()->position()));
        const QSizeF popupSize(popup->contentItem()->size());

        // ignore popup.{left,right}Padding() as not included in popup->contentItem()->size()
        // some styles prefer to draw the popup "over" (in z-axis direction) the combobox to hide
        // the combobox
        const bool styleDrawsPopupOverCombobox =
                comboBox->position().x() - popupSize.width() + comboBox->width() == popupPos.x();
        // some styles prefer to draw the popup right (in x-axis direction) of the combobox
        const bool styleDrawsPopupBelowCombobox =
                comboBox->position().x() - popupSize.width() - comboBox->leftPadding()
                == popupPos.x();

        QVERIFY(styleDrawsPopupOverCombobox || styleDrawsPopupBelowCombobox);
    }

    {
        QQuickComboBox *comboBox = window->findChild<QQuickComboBox *>("second");
        QVERIFY(comboBox);
        QQuickPopup *popup = comboBox->popup();
        QVERIFY(popup);
        popup->open();
        QTRY_COMPARE(popup->isVisible(), true);
        const QPointF popupPos(popup->contentItem()->mapToItem(comboBox->parentItem(),
                                                               popup->contentItem()->position()));

        // some styles prefer to draw the popup "over" (in z-axis direction) the combobox to hide
        // the combobox
        const bool styleDrawsPopupOverCombobox = comboBox->position().x() + comboBox->leftPadding()
                        + popup->leftPadding() + popup->rightPadding()
                == popupPos.x();
        // some styles prefer to draw the popup left (in y-axis direction) of the combobox
        const bool styleDrawsPopupAboveCombobox =
                comboBox->position().x() + comboBox->width() - comboBox->leftPadding()
                == popupPos.x();

        if (QQuickStyle::name() == QLatin1String("FluentWinUI3"))
            QEXPECT_FAIL("", "Rotated ComboBox is broken in FluentWinUI3 style", Abort);
        QVERIFY(styleDrawsPopupOverCombobox || styleDrawsPopupAboveCombobox);

        popup->close();
    }
}

void tst_QQuickPopup::focusMultiplePopup()
{
    QQuickApplicationHelper helper(this, "multiplepopup.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *rootItem = window->findChild<QQuickItem *>("rootItem");
    QTRY_VERIFY(rootItem->hasFocus());

    auto *buttonPopup = window->findChild<QQuickPopup *>("popup1");
    buttonPopup->open();
    QTRY_VERIFY(buttonPopup->isOpened());
    QVERIFY(!rootItem->hasFocus());
    QVERIFY(buttonPopup->hasFocus());

    auto *textEditPopup = window->findChild<QQuickPopup *>("popup2");
    textEditPopup->open();
    QTRY_VERIFY(textEditPopup->isOpened());
    QVERIFY(textEditPopup->hasFocus());

    auto *drawerPopup = window->findChild<QQuickPopup *>("popup3");
    drawerPopup->open();
    QTRY_VERIFY(drawerPopup->isVisible());
    QVERIFY(drawerPopup->hasFocus());

    auto *drawer = window->findChild<QQuickDrawer *>("drawer");
    drawer->close();
    QTRY_VERIFY(!drawer->isVisible());
    drawerPopup->close();
    QTRY_VERIFY(!drawerPopup->isVisible());
    QVERIFY(textEditPopup->hasFocus());

    textEditPopup->close();
    QTRY_VERIFY(!textEditPopup->isVisible());
    QVERIFY(buttonPopup->hasFocus());

    buttonPopup->close();
    QTRY_VERIFY(!buttonPopup->isVisible());

    QVERIFY(rootItem->hasFocus());
}

void tst_QQuickPopup::contentChildrenChange()
{
    QQmlEngine engine;
    QQmlComponent comp(&engine);
    comp.loadFromModule("QtQuick.Controls", "Popup");
    std::unique_ptr<QObject> root {comp.create()};
    QVERIFY(root);
    QQuickPopup *popup = qobject_cast<QQuickPopup *>(root.get());
    QVERIFY(popup);
    QSignalSpy spy(popup, &QQuickPopup::contentChildrenChanged);
    auto contentItem = std::make_unique<QQuickItem>();
    popup->setContentItem(contentItem.get());
    QCOMPARE(spy.count(), 1);
    auto newChild = std::make_unique<QQuickItem>();
    QQmlProperty contentItemChildren(contentItem.get());
    contentItemChildren.write(QVariant::fromValue(newChild.get()));
    QCOMPARE(spy.count(), 2);
}

void tst_QQuickPopup::doubleClickInMouseArea()
{
#ifdef Q_OS_ANDROID
    QSKIP("Test crashes. See QTBUG-118532");
#endif

    QQuickView window;
    QVERIFY(QQuickTest::showView(window, testFileUrl("doubleClickInMouseArea.qml")));

    auto *ma = window.rootObject()->findChild<QQuickMouseArea *>();
    QVERIFY(ma);
    QSignalSpy doubleClickSpy(ma, &QQuickMouseArea::doubleClicked);
    QSignalSpy longPressSpy(ma, &QQuickMouseArea::pressAndHold);
    QPoint p = ma->mapToScene(ma->boundingRect().center()).toPoint();

    // check with normal double click
    QTest::mouseDClick(&window, Qt::LeftButton, Qt::NoModifier, p);
    QCOMPARE(doubleClickSpy.count(), 1);

    // wait enough time for a wrong long press to happen
    QTest::qWait(QGuiApplication::styleHints()->mousePressAndHoldInterval() + 10);
    QCOMPARE(longPressSpy.count(), 0);
}

void tst_QQuickPopup::fadeDimmer_data()
{
    QTest::addColumn<bool>("modality");

    QTest::addRow("modal") << true;
    QTest::addRow("modeless") << false;
}

void tst_QQuickPopup::fadeDimmer()
{
    QFETCH(const bool, modality);
    QQuickApplicationHelper helper(this, "fadeDimmer.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);

    popup->setModal(modality);
    popup->open();
    auto dimmer = QQuickPopupPrivate::get(popup)->dimmer;
    QVERIFY(dimmer);
    int opacityChangeCount = 0;
    connect(dimmer, &QQuickItem::opacityChanged, this, [&opacityChangeCount]{
        ++opacityChangeCount;
    });
    QTRY_VERIFY(popup->isOpened());
    QTRY_COMPARE(dimmer->opacity(), popup->property("dimmerOpacity").toDouble());
    QCOMPARE_GT(opacityChangeCount, 2);

    opacityChangeCount = 0;
    popup->setVisible(false);
    QTRY_VERIFY(!popup->isVisible());
    QCOMPARE_GT(opacityChangeCount, 2);
}

void tst_QQuickPopup::noDimmer()
{
    QQuickApplicationHelper helper(this, "noDimmer.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *drawer = window->contentItem()->findChild<QQuickDrawer *>();
    QVERIFY(drawer);

    drawer->open();
    auto dimmer = QQuickPopupPrivate::get(drawer)->dimmer;
    QVERIFY(dimmer);
    // this must not crash
    QTRY_VERIFY(!drawer->isModal());
}

// The test verifies that press and release events for items that are ancestors of the overlay,
// but not a popup item, are not filtered by modal popups.
void tst_QQuickPopup::pointerEventsNotBlockedForNonPopupChildrenOfOverlayWithHigherZ()
{
    QQuickApplicationHelper helper(this, "parentToOverlay.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    QQuickMouseArea *lowerMouseArea = window->property("lowerMouseArea").value<QQuickMouseArea *>();
    QVERIFY(lowerMouseArea);
    QQuickMouseArea *upperMouseArea = window->property("upperMouseArea").value<QQuickMouseArea *>();
    QVERIFY(upperMouseArea);
    QQuickAbstractButton *button = window->property("button").value<QQuickAbstractButton *>();
    QVERIFY(button);

    QSignalSpy lowerMouseAreaSpy(lowerMouseArea, &QQuickMouseArea::clicked);
    QSignalSpy upperMouseAreaSpy(upperMouseArea, &QQuickMouseArea::clicked);
    QSignalSpy buttonSpy(button, &QQuickAbstractButton::clicked);

    popup->open();
    QTRY_VERIFY(popup->isOpened());

    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, button->mapToScene(button->boundingRect().center()).toPoint());

    // The event should have been consumed by the upperMouseArea,
    // since it's in the same hierarchy as the popup item, with a higher z.
    QTRY_COMPARE(upperMouseAreaSpy.count(), 1);
    QCOMPARE(lowerMouseAreaSpy.count(), 0);
    QCOMPARE(buttonSpy.count(), 0);

    upperMouseArea->setEnabled(false);

    QVERIFY(clickButton(button));
    // Since the upperMouseArea is disabled, the event should be sent to the button inside the popup.
    QCOMPARE(buttonSpy.count(), 1);
    QCOMPARE(lowerMouseAreaSpy.count(), 0);
    QCOMPARE(upperMouseAreaSpy.count(), 1);

    popup->close();
}

#define VERIFY_LOCAL_POS(POPUP, EXPECTED)                                                                     \
    QTRY_VERIFY2(qAbs(POPUP->x() - qreal(EXPECTED.x())) <= 1,                                                 \
        qPrintable(QStringLiteral("QQuickPopup::x() = %1, expected = %2").arg(POPUP->x()).arg(EXPECTED.x())));\
    QVERIFY2(qAbs(POPUP->y() - qreal(EXPECTED.y())) <= 1,                                                     \
        qPrintable(QStringLiteral("QQuickPopup::y() = %1, expected = %2").arg(POPUP->y()).arg(EXPECTED.y())))

#define VERIFY_GLOBAL_POS(FROM, POPUPWINDOW, EXPECTED)                                                             \
    QTRY_VERIFY2((POPUPWINDOW->position() - FROM->mapToGlobal(EXPECTED.x(), EXPECTED.y())).manhattanLength() <= 2, \
        qPrintable(QStringLiteral("PopupWindow pos = (%1, %2), expected (%3, %4)")                                 \
        .arg(POPUPWINDOW->x()).arg(POPUPWINDOW->y()).arg(EXPECTED.x()).arg(EXPECTED.y())))

void tst_QQuickPopup::popupWindowPositioning()
{
    if (!popupWindowsSupported)
        QSKIP("The platform doesn't support popup windows. Skipping test.");

    QQuickApplicationHelper helper(this, "simplepopup.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    QVERIFY(popupPrivate);

    popup->setPopupType(QQuickPopup::Window);

    QSignalSpy xSpy(popup, SIGNAL(xChanged()));
    QSignalSpy ySpy(popup, SIGNAL(yChanged()));

    popup->open();
    QTRY_VERIFY(popup->isOpened());

    QTRY_VERIFY(popupPrivate->popupWindow);
    auto *popupWindow = popupPrivate->popupWindow;
    QVERIFY(QTest::qWaitForWindowExposed(popupPrivate->popupWindow));
    QQuickTest::qWaitForPolish(popupPrivate->popupWindow);

    QTRY_COMPARE(xSpy.count(), 1);
    QTRY_COMPARE(ySpy.count(), 1);

    // x and y properties should be 50 initially (from simplepopup.qml)
    const QPoint initialPos(50, 50);

    if (QQuickStyle::name() == QLatin1String("Imagine"))
        QEXPECT_FAIL("", "This fails with the Imagine style: QTBUG-133530", Abort);
    VERIFY_GLOBAL_POS(popup->parentItem(), popupWindow, initialPos);
    VERIFY_LOCAL_POS(popup, initialPos);

    // Move popup via QQuickPopup API
    const QPoint secondPosition(100, 100);
    popup->setPosition(secondPosition.toPointF());

    QTRY_COMPARE(xSpy.count(), 2);
    QCOMPARE(ySpy.count(), 2);

    VERIFY_GLOBAL_POS(popup->parentItem(), popupWindow, secondPosition);
    VERIFY_LOCAL_POS(popup, secondPosition);

    // Move popup via QWindow API (which uses global coordinates)
    const QPoint thirdPosition(150, 150);
    popupWindow->setPosition(popup->parentItem()->mapToGlobal(thirdPosition.x(), thirdPosition.y()).toPoint());

    QTRY_COMPARE(xSpy.count(), 3);
    QCOMPARE(ySpy.count(), 3);

    VERIFY_GLOBAL_POS(popup->parentItem(), popupWindow, thirdPosition);
    VERIFY_LOCAL_POS(popup, thirdPosition);

    // Moving parent window should change the popups position (because it's stationary, but x and y are relative coordinates)
    const QPoint movement(30, 30);
    const QPoint oldPos = window->position();
    window->setPosition(oldPos + movement);

    QTRY_COMPARE(xSpy.count(), 4);
    QCOMPARE(ySpy.count(), 4);

    VERIFY_GLOBAL_POS(popup->parentItem(), popupWindow, (thirdPosition - movement));
    VERIFY_LOCAL_POS(popup, (thirdPosition - movement));

    // QTBUG-131098: Resizing the parent should not affect the popup's position.
    const QPoint finalPos = popup->position().toPoint();
    window->setWidth(window->width() - 10);

    QCOMPARE(xSpy.count(), 4);
    QCOMPARE(ySpy.count(), 4);

    VERIFY_GLOBAL_POS(popup->parentItem(), popupWindow, finalPos);
    VERIFY_LOCAL_POS(popup, finalPos);
}

void tst_QQuickPopup::popupWindowAnchorsCenterIn_data()
{
    QTest::addColumn<bool>("centerInParent");
    QTest::newRow("parent") << true;
    QTest::newRow("overlay") << false;
}

void tst_QQuickPopup::popupWindowAnchorsCenterIn()
{
    QFETCH(bool, centerInParent);

    if (!popupWindowsSupported)
        QSKIP("The platform doesn't support popup windows. Skipping test.");

    QQuickApplicationHelper helper(this, "popupCenterIn.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    QVERIFY(popupPrivate);

    popupPrivate->getAnchors()->setCenterIn(centerInParent ? window->contentItem() : QQuickOverlay::overlay(window));

    popup->open();
    QTRY_VERIFY(popup->isVisible());

    auto *popupWindow = popupPrivate->popupWindow;
    QVERIFY(popupWindow);

    const QPoint centeredPosition(qFloor(window->width() / 2 - popupWindow->width() / 2), qFloor(window->height() / 2 - popupWindow->height() / 2));

    if (QQuickStyle::name() == QLatin1String("Imagine"))
        QEXPECT_FAIL("", "This fails with the Imagine style: QTBUG-133530", Abort);
    VERIFY_GLOBAL_POS(popup->parentItem(), popupWindow, centeredPosition);
    VERIFY_LOCAL_POS(popup, centeredPosition);
}

void tst_QQuickPopup::popupWindowModality()
{
    QSKIP("The behavior isn't correctly implemented yet. Waiting for patch in qtbase");

    if (!popupWindowsSupported)
        QSKIP("The platform doesn't support popup windows. Skipping test.");

    QQuickApplicationHelper helper(this, "popupWithButtonInBackground.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);

    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    QVERIFY(popupPrivate);

    auto *button = window->findChild<QQuickButton *>();
    QVERIFY(button);

    QSignalSpy buttonSpy(button, SIGNAL(clicked()));

    popup->open();
    QTRY_VERIFY(popup->isVisible());

    auto *popupWindow = popupPrivate->popupWindow;
    QVERIFY(popupWindow);
    QVERIFY(popupWindow->isVisible());
    // NonModal by default
    QCOMPARE(popupWindow->modality(), Qt::NonModal);

    // Non modal popups should close on press outside
    QTest::mouseClick(helper.window, Qt::LeftButton, Qt::NoModifier, button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint());
    QTRY_COMPARE(buttonSpy.count(), 1);
    QVERIFY(!popupWindow->isVisible());

    popup->setModal(true);
    popup->open();
    QTRY_VERIFY(popup->isVisible());
    QVERIFY(popupWindow->isVisible());
    QCOMPARE(popupWindow->modality(), Qt::ApplicationModal);

    // Pressing outside the popup shouldn't cause the button to get the event, because of modality.
    QTest::mouseClick(helper.window, Qt::LeftButton, Qt::NoModifier, button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint());
    QCoreApplication::processEvents();
    QCOMPARE(buttonSpy.count(), 1);
    QVERIFY(popupWindow->isVisible());

    popup->close();
    QTRY_VERIFY(!popup->isVisible());
}

void tst_QQuickPopup::popupWindowClosesOnParentWindowClosing()
{
    QSKIP("The behavior isn't correctly implemented yet. Waiting for patch in qtbase");
    if (!popupWindowsSupported)
        QSKIP("The platform doesn't support popup windows. Skipping test.");

    QQuickApplicationHelper helper(this, "simplepopup.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    QVERIFY(popupPrivate);

    popup->setPopupType(QQuickPopup::Window);

    popup->open();
    QTRY_VERIFY(popup->isVisible());

    auto *popupWindow = popupPrivate->popupWindow;
    QVERIFY(popupWindow);
    QVERIFY(popupWindow->isVisible());

    // Closing parent window, should close child window;
    window->close();

    QTRY_VERIFY(!window->isVisible());
    QTRY_VERIFY(!popupWindow->isVisible());
}

void tst_QQuickPopup::popupWindowClosingPolicy()
{
    if (!popupWindowsSupported)
        QSKIP("The platform doesn't support popup windows. Skipping test.");

    QQuickApplicationHelper helper(this, "simplepopup.qml");
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    const QRect screenGeometry = window->screen()->availableGeometry();
    const QPoint offset = QPoint(window->width() / 2, window->height() / 2);
    window->setFramePosition(screenGeometry.center() - offset);

    const QPoint outsideWindow = window->geometry().topRight() + QPoint(100, 100);

    QQuickVisualTestUtils::moveMouseAway(window);

    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    QVERIFY(popupPrivate);

    popup->setPopupType(QQuickPopup::Window);
    popup->setClosePolicy(QQuickPopup::CloseOnEscape);

    popup->open();
    QTRY_VERIFY(popup->isOpened());

    auto *popupWindow = popupPrivate->popupWindow;
    QVERIFY(popupWindow);
    QVERIFY(popupWindow->isVisible());

    // Escape should work like normal.
    QTest::keyClick(window, Qt::Key_Escape);

    QTRY_VERIFY(!popup->isVisible());
    QVERIFY(!popupWindow->isVisible());

    // Test MousePress events
    popup->setClosePolicy(QQuickPopup::CloseOnPressOutside);

    popup->open();
    QTRY_VERIFY(popup->isOpened());
    QVERIFY(QTest::qWaitForWindowExposed(popupWindow));

    // Should not close, since point is inside the popup window.
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, QPoint(popup->x() + popup->width() / 2, popup->y() + popup->height() / 2));

    QQuickTest::qWaitForPolish(popupWindow);
    QVERIFY(popup->isVisible());
    QVERIFY(popupWindow->isVisible());

    // Releasing outside: should not close due to closePolicy.
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, outsideWindow);

    QQuickTest::qWaitForPolish(popupWindow);
    QVERIFY(popup->isVisible());
    QVERIFY(popupWindow->isVisible());

    // Pressing outside: should close!
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, outsideWindow);

    QTRY_VERIFY(!popup->isVisible());
    QVERIFY(!popupWindow->isVisible());

    // Test MouseRelease events
    popup->setClosePolicy(QQuickPopup::CloseOnReleaseOutside);

    popup->open();
    QTRY_VERIFY(popup->isOpened());
    QVERIFY(QTest::qWaitForWindowExposed(popupWindow));

    // Should not close, since the point is inside the popup window.
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, QPoint(popup->x() + popup->width() / 2, popup->y() + popup->height() / 2));

    QQuickTest::qWaitForPolish(popupWindow);
    QVERIFY(popup->isVisible());
    QVERIFY(popupWindow->isVisible());

    // Pressing outside: should not close due to closePolicy.
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, outsideWindow);

    QQuickTest::qWaitForPolish(popupWindow);
    QVERIFY(popup->isVisible());
    QVERIFY(popupWindow->isVisible());

    // Releasing outside: should close!
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, outsideWindow);

    QTRY_VERIFY(!popup->isVisible());
    QVERIFY(!popupWindow->isVisible());
}

void tst_QQuickPopup::initialPopupSize_data()
{
    QTest::addColumn<QQuickPopup::PopupType>("popupType");

    QTest::newRow("Item") << QQuickPopup::Item;
    if (popupWindowsSupported)
        QTest::newRow("Window") << QQuickPopup::Window;
}

void tst_QQuickPopup::initialPopupSize()
{
    QFETCH(QQuickPopup::PopupType, popupType);
    QQuickApplicationHelper helper(this, "reparentingPopup.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    QVERIFY(popupPrivate);

    popup->setPopupType(popupType);
    popup->open();
    QTRY_VERIFY(popup->isOpened());

    QCOMPARE(popup->width(), 200);
    QCOMPARE(popup->height(), 200);

    auto popupItem = popupPrivate->popupItem;
    QCOMPARE(popupItem->width(), 200);
    QCOMPARE(popupItem->height(), 200);

    if (popupType == QQuickPopup::Window) {
        auto *popupWindow = popupPrivate->popupWindow;
        QVERIFY(popupWindow);
        QVERIFY(popupWindow->isVisible());
        if (QQuickStyle::name() == QLatin1String("Imagine"))
            QEXPECT_FAIL("", "This fails with the Imagine style: QTBUG-133530", Abort);
        QCOMPARE(popupWindow->width(), 200);
        QCOMPARE(popupWindow->height(), 200);
    }
}

void tst_QQuickPopup::popupWindowChangingParent()
{
    if (!popupWindowsSupported)
        QSKIP("The platform doesn't support popup windows. Skipping test.");

    QQuickApplicationHelper helper(this, "reparentingPopup.qml");
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    QVERIFY(popupPrivate);

    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));
    QVERIFY(!popup->isVisible());

    QQuickItem *item1 = window->property("rectangle1").value<QQuickItem *>();
    QVERIFY(item1);

    QQuickItem *item2 = window->property("rectangle2").value<QQuickItem *>();
    QVERIFY(item2);

    QQuickItem *item3 = window->property("rectangle3").value<QQuickItem *>();
    QVERIFY(item3);

    popup->open();
    QTRY_VERIFY(popup->isOpened());

    QTRY_VERIFY(popupPrivate->popupWindow);
    QWindow *popupWindow = popupPrivate->popupWindow;

    QTRY_VERIFY(popupWindow->isVisible());
    QVERIFY(QTest::qWaitForWindowExposed(popupWindow));

    const QPoint initialPos(10, 10);

    if (QQuickStyle::name() == QLatin1String("Imagine"))
        QEXPECT_FAIL("", "This fails with the Imagine style: QTBUG-133530", Abort);
    VERIFY_GLOBAL_POS(item1, popupWindow, initialPos);
    VERIFY_LOCAL_POS(popup, initialPos);

    popup->setParentItem(item1);

    VERIFY_GLOBAL_POS(item1, popupWindow, initialPos);
    VERIFY_LOCAL_POS(popup, initialPos);

    popup->setParentItem(item2);

    VERIFY_GLOBAL_POS(item2, popupWindow, initialPos);
    VERIFY_LOCAL_POS(popup, initialPos);

    popup->setParentItem(item3);

    VERIFY_GLOBAL_POS(item3, popupWindow, initialPos);
    VERIFY_LOCAL_POS(popup, initialPos);

    popup->close();
}

void tst_QQuickPopup::popupWindowChangingParentWindow()
{
    if (!popupWindowsSupported)
        QSKIP("The platform doesn't support popup windows. Skipping test.");

    QQuickApplicationHelper helper(this, "reparentingPopupToDifferentWindows.qml");
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    QVERIFY(popupPrivate);

    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));
    QVERIFY(!popup->isVisible());

    QQuickWindow *childWindow = window->property("childWindow").value<QQuickWindow *>();
    QVERIFY(childWindow);

    childWindow->show();
    QVERIFY(QTest::qWaitForWindowExposed(childWindow));
    QVERIFY(!popup->isVisible());

    popup->open();
    QTRY_VERIFY(popup->isOpened());
    QTRY_VERIFY(popupPrivate->popupWindow);
    auto *popupWindow = popupPrivate->popupWindow;
    QVERIFY(QTest::qWaitForWindowExposed(popupWindow));
    QQuickTest::qWaitForPolish(popupWindow);

    QCOMPARE(popup->parentItem(), window->contentItem());
    // The expected value is 0, but we allow for 1 pixel of leniency,
    // similar to VERIFY_GLOBAL_POS.
    QCOMPARE_LT(qAbs(popup->x()), 2);
    QCOMPARE_LT(qAbs(popup->y()), 2);

    popup->close();
    QTRY_VERIFY(!popup->isVisible());
    popup->setParentItem(childWindow->contentItem());
    popup->open();
    QTRY_VERIFY(popup->isOpened());
    QVERIFY(QTest::qWaitForWindowExposed(popupWindow));
    QQuickTest::qWaitForPolish(popupWindow);
    QCOMPARE(popup->parentItem(), childWindow->contentItem());

    QSignalSpy windowMoveSpy(window, &QWindow::yChanged);
    window->setY(window->y() + 100);
    QTRY_COMPARE(windowMoveSpy.count(), 1);
    QCOMPARE_LT(qAbs(popup->x()), 2);
    QCOMPARE_LT(qAbs(popup->y()), 2);

    popup->close();
    QTRY_VERIFY(!popup->isVisible());
}

void tst_QQuickPopup::popupWindowFocus()
{
#if defined(Q_OS_QNX)
    QSKIP("This test doesn't pass on QNX. It needs more investigation before it can be enabled");
#endif

    if (!popupWindowsSupported)
        QSKIP("The platform doesn't support popup windows. Skipping test.");

    QQuickApplicationHelper helper(this, "popupWindowFocusHandling.qml");
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    QVERIFY(window);
    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    QVERIFY(popupPrivate);
    QQuickTextInput *textField1 = window->property("textField1").value<QQuickTextInput *>();
    QVERIFY(textField1);
    QQuickTextInput *textField2 = window->property("textField2").value<QQuickTextInput *>();
    QVERIFY(textField2);

    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window));
    QVERIFY(QQuickTest::qWaitForPolish(window));
    QVERIFY(QGuiApplication::focusObject() == textField1);
    QVERIFY(window->focusObject() == textField1);
    QVERIFY(textField1->hasActiveFocus());

    QTest::keyClick(window, Qt::Key_Q);
    QTRY_COMPARE(textField1->text(), "q");
    popup->open();
    QTRY_VERIFY(popup->isOpened());
    QTRY_VERIFY(popupPrivate->popupWindow);
    auto *popupWindow = popupPrivate->popupWindow;
    QVERIFY(popupWindow->isVisible());
    QCOMPARE(QGuiApplicationPrivate::popupCount(), 1);
    // The focusWindow should still be the main window,
    // the popup window should get its event forwarded via the delivery agent
    QVERIFY(QGuiApplication::focusWindow() == window);
    QVERIFY(popupWindow->focusObject() == textField2);
    QVERIFY(QTest::qWaitForWindowActive(popupWindow));
    QVERIFY(QQuickTest::qWaitForPolish(popupWindow));
    QTest::keyClick(window, Qt::Key_T);
    QVERIFY(QQuickTest::qWaitForPolish(textField2));
    QTRY_COMPARE(textField2->text(), "t");
    popup->close();
    QTRY_VERIFY(!popup->isOpened());
    QTRY_VERIFY(!popup->isVisible());
    QVERIFY(QGuiApplication::focusObject() == textField1);
    QCOMPARE(QGuiApplicationPrivate::popupCount(), 0);

    // If the popup doesn't have focus, it shouldn't receive key events.
    popup->setFocus(false);
    popup->open();
    QTRY_VERIFY(popup->isOpened());
    QVERIFY(popupWindow->isVisible());
    QCOMPARE(QGuiApplicationPrivate::popupCount(), 1);
    QVERIFY(QTest::qWaitForWindowActive(popupWindow));
    QVERIFY(QQuickTest::qWaitForPolish(popupWindow));
    QTest::keyClick(window, Qt::Key_T);
    QVERIFY(QQuickTest::qWaitForPolish(textField1));
    QTRY_COMPARE(textField1->text(), "qt");
    popup->close();
    QTRY_VERIFY(!popup->isOpened());
    QTRY_VERIFY(!popup->isVisible());
}

void tst_QQuickPopup::popupTypeChangeFromWindowToItem()
{
    if (!popupWindowsSupported)
        QSKIP("The platform doesn't support native popup windows. Skipping test.");

    QQuickApplicationHelper helper(this, "simplepopup.qml");
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));
    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    QVERIFY(popupPrivate);

    QQuickOverlay *overlay = QQuickOverlay::overlay(window);
    QVERIFY(overlay);

    popup->setPopupType(QQuickPopup::Window);

    popup->open();
    QTRY_VERIFY(popup->isOpened());
    const QWindow *popupWindow = popupPrivate->popupWindow;
    QVERIFY(popupWindow);
    QTRY_VERIFY(popupWindow->isVisible());
    if (QQuickStyle::name() == QLatin1String("Imagine"))
        QEXPECT_FAIL("", "This fails with the Imagine style: QTBUG-133530", Abort);
    QCOMPARE(popupPrivate->popupItem->position(), QPointF(0, 0));
    QVERIFY(!overlay->childItems().contains(popup->popupItem()));

    popup->close();
    QTRY_VERIFY(!popup->isVisible());
    QVERIFY(!popupWindow->isVisible());

    popup->setPopupType(QQuickPopup::Item);

    popup->open();
    QTRY_VERIFY(popup->isOpened());
    QVERIFY(!popupWindow || !popupWindow->isVisible());
    QCOMPARE(popupPrivate->popupItem->position(), QPointF(50, 50));
    QVERIFY(overlay->childItems().contains(popup->popupItem()));

    popup->close();
}

void tst_QQuickPopup::popupTypeChangeFromItemToWindow()
{
    if (!popupWindowsSupported)
        QSKIP("The platform doesn't support popup windows. Skipping test.");

    QQuickApplicationHelper helper(this, "simplepopup.qml");
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));
    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    QVERIFY(popupPrivate);

    QQuickOverlay *overlay = QQuickOverlay::overlay(window);
    QVERIFY(overlay);

    popup->setPopupType(QQuickPopup::Item);

    popup->open();
    QTRY_VERIFY(popup->isOpened());
    QVERIFY(!popupPrivate->popupWindow);
    QCOMPARE(popupPrivate->popupItem->position(), QPointF(50, 50));
    QVERIFY(overlay->childItems().contains(popup->popupItem()));

    popup->close();
    QTRY_VERIFY(!popup->isVisible());

    popup->setPopupType(QQuickPopup::Window);

    popup->open();
    QTRY_VERIFY(popup->isOpened());
    const QWindow *popupWindow = popupPrivate->popupWindow;
    QVERIFY(popupWindow);
    QTRY_VERIFY(popupWindow->isVisible());
    if (QQuickStyle::name() == QLatin1String("Imagine"))
        QEXPECT_FAIL("", "This fails with the Imagine style: QTBUG-133530", Abort);
    QCOMPARE(popupPrivate->popupItem->position(), QPointF(0, 0));
    QVERIFY(!overlay->childItems().contains(popup->popupItem()));

    popup->close();
}

void tst_QQuickPopup::resetHoveredStateForItemsWithinPopup()
{
    QQuickControlsApplicationHelper helper(this, "resetHoveredForItemsWithinOverlay.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickPopup *controlsPopup = window->property("controlsPopup").value<QQuickPopup*>();
    QVERIFY(controlsPopup);

    QQuickPopup *blockInputPopup = window->property("blockInputPopup").value<QQuickPopup*>();
    QVERIFY(controlsPopup);

    controlsPopup->open();
    QTRY_VERIFY(controlsPopup->isOpened());

    QPoint moveStart(window->width() / 2, window->height() / 2);
    QPoint moveEnd(moveStart.x() + 1 , moveStart.y() + 1);
    QTest::mouseMove(window, moveStart);
    QTest::mouseMove(window, moveEnd);

    auto *controlItem = qobject_cast<QQuickControl *>(controlsPopup->contentItem()->childItems().at(0));
    QVERIFY(controlItem);

    // Check hover enabled for the control item within the popup
    QTRY_VERIFY(controlItem->isHovered());

    // Open the modal popup window over the existing control item
    blockInputPopup->open();
    QTRY_VERIFY(blockInputPopup->isOpened());

    // Control item hovered shall be disabled once we open the modal popup
    QTRY_VERIFY(!controlItem->isHovered());

    blockInputPopup->close();
    QTRY_VERIFY(!blockInputPopup->isOpened());
    QTRY_VERIFY(!blockInputPopup->isVisible());

    // Control item hovered is re-enabled after the modal popup is closed
    QTRY_VERIFY(controlItem->isHovered());

    controlsPopup->close();
    QTRY_VERIFY(!controlsPopup->isOpened());
    QTRY_VERIFY(!controlsPopup->isVisible());
}

void tst_QQuickPopup::noInfiniteRecursionOnParentWindowDestruction()
{
    QQuickControlsApplicationHelper helper(this, "simplepopup.qml");
    QVERIFY2(helper.ready, helper.failureMessage());
    auto *popup = helper.window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    popup->setPopupType(QQuickPopup::PopupType::Window);
    // The popup won't actually open, since the parent window is still invisible.
    popup->open();
    QTRY_VERIFY(popup->isOpened());

    // Dont crash on destruction
}

void tst_QQuickPopup::popupWindowDestructedBeforeQQuickPopup()
{
    if (!popupWindowsSupported)
        QSKIP("The platform doesn't support popup windows. Skipping test.");

    QQuickControlsApplicationHelper helper(this, "simplepopup.qml");
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    QVERIFY(window);

    auto *popup = helper.window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    popup->setPopupType(QQuickPopup::PopupType::Window);

    window->show();
    window->requestActivate();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    popup->open();
    QTRY_VERIFY(popup->isOpened());
    QTRY_VERIFY(popupPrivate->popupWindow);

    QSignalSpy popupDestroyedSpy(popup, SIGNAL(destroyed()));
    QVERIFY(popupDestroyedSpy.isValid());

    QSignalSpy popupWindowDestroyedSpy(popupPrivate->popupWindow, SIGNAL(destroyed()));
    QVERIFY(popupWindowDestroyedSpy.isValid());

    bool lambdaExecuted = false;

    connect(popupPrivate->popupWindow, &QObject::destroyed, [&popupDestroyedSpy, &lambdaExecuted]() {
        // Check that the popup window has been destroyed before the popup has been destroyed.
        // The events come in the same frame, so we can't just use QTRY_COMPARE.
        QCOMPARE(popupDestroyedSpy.size(), 0);

        lambdaExecuted = true;
    });

    popup->deleteLater();
    QTRY_VERIFY(lambdaExecuted);

    QCOMPARE(popupDestroyedSpy.size(), 1);
    QCOMPARE(popupWindowDestroyedSpy.size(), 1);

    // Check geometry changes don't cause a crash from hanging connections
    window->setX(10);
    window->hide();

    // Doesn't crash on destruction
}

void tst_QQuickPopup::popupWindowPositionerRespectingScreenBounds_data()
{
    QTest::addColumn<QQuickPopup::PopupType>("popupType");
    QTest::newRow("Popup.Item") << QQuickPopup::Item;
    if (popupWindowsSupported &&
        QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::WindowActivation))
        QTest::newRow("Popup.Window") << QQuickPopup::Window;
}

void tst_QQuickPopup::popupWindowPositionerRespectingScreenBounds()
{
    QFETCH(QQuickPopup::PopupType, popupType);

    QQuickApplicationHelper helper(this, "simplepopup.qml");
    QVERIFY2(helper.ready, helper.failureMessage());
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    auto *popup = window->contentItem()->findChild<QQuickPopup *>();
    QVERIFY(popup);
    auto *popupPrivate = QQuickPopupPrivate::get(popup);
    QVERIFY(popupPrivate);

    popup->setPopupType(popupType);
    popup->open();
    QTRY_VERIFY(popup->isOpened());

    class AbstractBoundsFinder {
    public:
        virtual QRectF operator()() const = 0;
        virtual ~AbstractBoundsFinder() = default;
    };

    std::unique_ptr<AbstractBoundsFinder> abf;

    if (popupPrivate->usePopupWindow()) {
        popupPrivate->popupWindow->requestActivate();
        QVERIFY(QTest::qWaitForWindowActive(popupPrivate->popupWindow));
        QCOMPARE(popupPrivate->popupWindow, popup->contentItem()->window());
        QVERIFY(QQuickTest::qWaitForPolish(popupPrivate->popupWindow));

        class ScreenBoundsFinder : public AbstractBoundsFinder {
        public:
            explicit ScreenBoundsFinder(QScreen *screen)
                : m_screen(screen) {}
            QRectF operator()() const override {
                return m_screen->availableGeometry().toRectF();
            }
        private:
            QScreen *m_screen = nullptr;
        };

        QScreen *screen = QGuiApplication::primaryScreen();
        QVERIFY(screen);

        abf.reset(new ScreenBoundsFinder(screen));
    } else {
        popupPrivate->allowHorizontalFlip = true;
        popupPrivate->allowVerticalFlip = true;
        popupPrivate->allowHorizontalMove = true;
        popupPrivate->allowVerticalMove = true;

        class WindowBoundsFinder : public AbstractBoundsFinder {
        public:
            explicit WindowBoundsFinder(QWindow *win)
                : m_window(win) {}
            QRectF operator()() const override {
                return QRectF (m_window->x(), m_window->y(), m_window->width(), m_window->height());
            }
        private:
            QWindow *m_window = nullptr;
        };

        abf.reset(new WindowBoundsFinder(window));
    }

    const QPointF positionOutsideTopLeftBound = (*abf)().bottomRight() * -1;
    const QPointF positionOutsideBottomRightBound = (*abf)().bottomRight() * 2;

    QSignalSpy xSpy(popup, &QQuickPopup::xChanged);
    QSignalSpy ySpy(popup, &QQuickPopup::yChanged);

    popup->setX(positionOutsideTopLeftBound.x());
    QTRY_COMPARE(xSpy.count(), 1);
    QVERIFY(QQuickTest::qWaitForPolish(window));
    if (QQuickStyle::name() == QLatin1String("Imagine") && popupType == QQuickPopup::Window)
        QEXPECT_FAIL("", "This fails with the Imagine style: QTBUG-133530", Abort);
    QTRY_VERIFY2(qAbs(window->contentItem()->mapToGlobal(popup->x(), popup->y()).x() - (*abf)().left()) < 2,
                 qPrintable(QStringLiteral("Expected popup's x position to be %1 but it's %2")
                            .arg((*abf)().left()).arg(window->contentItem()->mapToGlobal(popup->x(), popup->y()).x())));

    popup->setY(positionOutsideTopLeftBound.y());
    QTRY_COMPARE(ySpy.count(), 1);
    QVERIFY(QQuickTest::qWaitForPolish(window));
    QTRY_VERIFY2(qAbs(window->contentItem()->mapToGlobal(popup->x(), popup->y()).y() - (*abf)().top()) < 2,
                 qPrintable(QStringLiteral("Expected popup's y position to be %1 but it's %2")
                            .arg((*abf)().top()).arg(window->contentItem()->mapToGlobal(popup->x(), popup->y()).y())));

    popup->setX(positionOutsideBottomRightBound.x());
    QTRY_COMPARE(xSpy.count(), 2);
    QVERIFY(QQuickTest::qWaitForPolish(window));
    QTRY_VERIFY2(qAbs(window->contentItem()->mapToGlobal(popup->x(), popup->y()).x() - ((*abf)().right() - popup->width())) < 2,
                 qPrintable(QStringLiteral("Expected popup's x position to be %1 but it's %2")
                            .arg((*abf)().right() - popup->width()).arg(window->contentItem()->mapToGlobal(popup->x(), popup->y()).x())));

    popup->setY(positionOutsideBottomRightBound.y());
    QTRY_COMPARE(ySpy.count(), 2);
    QVERIFY(QQuickTest::qWaitForPolish(window));
    QTRY_VERIFY2(qAbs(window->contentItem()->mapToGlobal(popup->x(), popup->y()).y() - ((*abf)().bottom() - popup->height())) < 2,
                 qPrintable(QStringLiteral("Expected popup's y position to be %1 but it's %2")
                            .arg((*abf)().bottom() - popup->height()).arg(window->contentItem()->mapToGlobal(popup->x(), popup->y()).y())));

    popup->close();
}

QTEST_QUICKCONTROLS_MAIN(tst_QQuickPopup)

#include "tst_qquickpopup.moc"
