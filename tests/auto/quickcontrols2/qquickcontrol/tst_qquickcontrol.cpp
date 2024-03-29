/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/qtest.h>
#include <QtTest/qsignalspy.h>
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QtQuickTestUtils/private/visualtestutils_p.h>
#include <QtQuickTemplates2/private/qquickbutton_p.h>
#include <QtQuickControlsTestUtils/private/qtest_quickcontrols_p.h>
#include <QtQuick/private/qquicktext_p_p.h>

using namespace QQuickVisualTestUtils;

class tst_QQuickControl : public QQmlDataTest
{
    Q_OBJECT

public:
    tst_QQuickControl();

private slots:
    void initTestCase() override;
    void flickable();
    void fractionalFontSize();
    void resizeBackgroundKeepsBindings();

private:
    QScopedPointer<QPointingDevice> touchDevice;
};

tst_QQuickControl::tst_QQuickControl()
    : QQmlDataTest(QT_QMLTEST_DATADIR)
{
}

void tst_QQuickControl::initTestCase()
{
    QQmlDataTest::initTestCase();
    qputenv("QML_NO_TOUCH_COMPRESSION", "1");

    touchDevice.reset(QTest::createTouchDevice());
}

void tst_QQuickControl::flickable()
{
    // Check that when a Button that is inside a Flickable with a pressDelay
    // still gets the released and clicked signals sent due to the fact that
    // Flickable sends a mouse event for the delay and not a touch event
    QQuickApplicationHelper helper(this, QStringLiteral("flickable.qml"));
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QQuickButton *button = window->property("button").value<QQuickButton *>();
    QVERIFY(button);

    QSignalSpy buttonPressedSpy(button, SIGNAL(pressed()));
    QVERIFY(buttonPressedSpy.isValid());

    QSignalSpy buttonReleasedSpy(button, SIGNAL(released()));
    QVERIFY(buttonReleasedSpy.isValid());

    QSignalSpy buttonClickedSpy(button, SIGNAL(clicked()));
    QVERIFY(buttonClickedSpy.isValid());

    QTest::touchEvent(window, touchDevice.data()).press(0, QPoint(button->width() / 2, button->height() / 2));
    QTRY_COMPARE(buttonPressedSpy.count(), 1);
    QTest::touchEvent(window, touchDevice.data()).release(0, QPoint(button->width() / 2, button->height() / 2));
    QTRY_COMPARE(buttonReleasedSpy.count(), 1);
    QTRY_COMPARE(buttonClickedSpy.count(), 1);
}

void tst_QQuickControl::fractionalFontSize()
{
    QQuickApplicationHelper helper(this, QStringLiteral("fractionalFontSize.qml"));
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));
    const QQuickControl *control = window->property("control").value<QQuickControl *>();
    QVERIFY(control);
    QQuickText *contentItem = qobject_cast<QQuickText *>(control->contentItem());
    QVERIFY(contentItem);

    QVERIFY(!contentItem->truncated());

    QVERIFY2(qFuzzyCompare(contentItem->contentWidth(),
            QQuickTextPrivate::get(contentItem)->layout.boundingRect().width()),
            "The QQuickText::contentWidth() doesn't match the layout's preferred text width");
}

void tst_QQuickControl::resizeBackgroundKeepsBindings()
{
    QTest::ignoreMessage(
            QtMsgType::QtInfoMsg,
            QRegularExpression("Binding on background is not deferred .*"));
    QQuickApplicationHelper helper(this, QStringLiteral("resizeBackgroundKeepsBindings.qml"));
    QQuickWindow *window = helper.window;
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));
    auto ctxt = qmlContext(window);
    QVERIFY(ctxt);
    auto background = qobject_cast<QQuickItem *>(ctxt->objectForName("background"));
    QVERIFY(background);
    QCOMPARE(background->height(), 4);
    QVERIFY(background->bindableHeight().hasBinding());
}

QTEST_QUICKCONTROLS_MAIN(tst_QQuickControl)

#include "tst_qquickcontrol.moc"
