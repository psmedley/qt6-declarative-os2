/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
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

#ifndef QQUICKSCROLLBAR_P_P_H
#define QQUICKSCROLLBAR_P_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuickTemplates2/private/qquickscrollbar_p.h>
#include <QtQuickTemplates2/private/qquickcontrol_p_p.h>
#include <QtQuick/private/qquickitemchangelistener_p.h>

QT_BEGIN_NAMESPACE

class QQuickFlickable;
class QQuickIndicatorButton;

class QQuickScrollBarPrivate : public QQuickControlPrivate
{
    Q_DECLARE_PUBLIC(QQuickScrollBar)

public:
    static QQuickScrollBarPrivate *get(QQuickScrollBar *bar)
    {
        return bar->d_func();
    }

    struct VisualArea
    {
        VisualArea(qreal pos, qreal sz)
            : position(pos), size(sz) { }
        qreal position = 0;
        qreal size = 0;
    };
    VisualArea visualArea() const;

    qreal logicalPosition(qreal position) const;

    qreal snapPosition(qreal position) const;
    qreal positionAt(const QPointF &point) const;
    void setInteractive(bool interactive);
    void updateActive();
    void resizeContent() override;
    void itemImplicitWidthChanged(QQuickItem *item) override;
    void itemImplicitHeightChanged(QQuickItem *item) override;

    void handlePress(const QPointF &point) override;
    void handleMove(const QPointF &point) override;
    void handleRelease(const QPointF &point) override;
    void handleUngrab() override;

    void visualAreaChange(const VisualArea &newVisualArea, const VisualArea &oldVisualArea);

    void updateHover(const QPointF &pos,  std::optional<bool> newHoverState = {});

    QQuickIndicatorButton *decreaseVisual = nullptr;
    QQuickIndicatorButton *increaseVisual = nullptr;
    qreal size = 0;
    qreal position = 0;
    qreal stepSize = 0;
    qreal offset = 0;
    qreal minimumSize = 0;
    bool active = false;
    bool pressed = false;
    bool moving = false;
    bool interactive = true;
    bool explicitInteractive = false;
    Qt::Orientation orientation = Qt::Vertical;
    QQuickScrollBar::SnapMode snapMode = QQuickScrollBar::NoSnap;
    QQuickScrollBar::Policy policy = QQuickScrollBar::AsNeeded;
};

class QQuickScrollBarAttachedPrivate : public QObjectPrivate, public QQuickItemChangeListener
{
    Q_DECLARE_PUBLIC(QQuickScrollBarAttached)

public:
    static QQuickScrollBarAttachedPrivate *get(QQuickScrollBarAttached *attached)
    {
        return attached->d_func();
    }

    void setFlickable(QQuickFlickable *flickable);

    void initHorizontal();
    void initVertical();
    void cleanupHorizontal();
    void cleanupVertical();
    void activateHorizontal();
    void activateVertical();
    void scrollHorizontal();
    void scrollVertical();
    void mirrorVertical();

    void layoutHorizontal(bool move = true);
    void layoutVertical(bool move = true);

    void itemGeometryChanged(QQuickItem *item, QQuickGeometryChange change, const QRectF &diff) override;
    void itemImplicitWidthChanged(QQuickItem *item) override;
    void itemImplicitHeightChanged(QQuickItem *item) override;
    void itemDestroyed(QQuickItem *item) override;

    QQuickFlickable *flickable = nullptr;
    QQuickScrollBar *horizontal = nullptr;
    QQuickScrollBar *vertical = nullptr;
};

QT_END_NAMESPACE

#endif // QQUICKSCROLLBAR_P_P_H
