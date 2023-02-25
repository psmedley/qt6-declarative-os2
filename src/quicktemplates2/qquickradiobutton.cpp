/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickradiobutton_p.h"
#include "qquickcontrol_p_p.h"
#include "qquickabstractbutton_p_p.h"

#include <QtGui/qpa/qplatformtheme.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype RadioButton
    \inherits AbstractButton
//!     \instantiates QQuickRadioButton
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup qtquickcontrols2-buttons
    \brief Exclusive radio button that can be toggled on or off.

    \image qtquickcontrols2-radiobutton.gif

    RadioButton presents an option button that can be toggled on (checked) or
    off (unchecked). Radio buttons are typically used to select one option
    from a set of options.

    RadioButton inherits its API from \l AbstractButton. For instance,
    you can set \l {AbstractButton::text}{text} and react to
    \l {AbstractButton::clicked}{clicks} using the AbstractButton API.
    The state of the radio button can be set with the
    \l {AbstractButton::}{checked} property.

    Radio buttons are \l {AbstractButton::autoExclusive}{auto-exclusive}
    by default. Only one button can be checked at any time amongst radio
    buttons that belong to the same parent item; checking another button
    automatically unchecks the previously checked one. For radio buttons
    that do not share a common parent, ButtonGroup can be used to manage
    exclusivity.

    \l RadioDelegate is similar to RadioButton, except that it is typically
    used in views.

    \code
    ColumnLayout {
        RadioButton {
            checked: true
            text: qsTr("First")
        }
        RadioButton {
            text: qsTr("Second")
        }
        RadioButton {
            text: qsTr("Third")
        }
    }
    \endcode

    \sa ButtonGroup, {Customizing RadioButton}, {Button Controls}, RadioDelegate
*/

class Q_QUICKTEMPLATES2_PRIVATE_EXPORT QQuickRadioButtonPrivate : public QQuickAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(QQuickRadioButton)

public:
    QPalette defaultPalette() const override { return QQuickTheme::palette(QQuickTheme::RadioButton); }
};

QQuickRadioButton::QQuickRadioButton(QQuickItem *parent)
    : QQuickAbstractButton(*(new QQuickRadioButtonPrivate), parent)
{
    setCheckable(true);
    setAutoExclusive(true);
}

QFont QQuickRadioButton::defaultFont() const
{
    return QQuickTheme::font(QQuickTheme::RadioButton);
}

#if QT_CONFIG(accessibility)
QAccessible::Role QQuickRadioButton::accessibleRole() const
{
    return QAccessible::RadioButton;
}
#endif

QT_END_NAMESPACE

#include "moc_qquickradiobutton_p.cpp"
