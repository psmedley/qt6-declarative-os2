// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWIDGETPLATFORMCOLORDIALOG_P_H
#define QWIDGETPLATFORMCOLORDIALOG_P_H

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

#include <QtGui/qpa/qplatformdialoghelper.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QColorDialog;

class QWidgetPlatformColorDialog : public QPlatformColorDialogHelper
{
    Q_OBJECT

public:
    explicit QWidgetPlatformColorDialog(QObject *parent = nullptr);
    ~QWidgetPlatformColorDialog();

    QColor currentColor() const override;
    void setCurrentColor(const QColor &color) override;

    void exec() override;
    bool show(Qt::WindowFlags flags, Qt::WindowModality modality, QWindow *parent) override;
    void hide() override;

private:
    QScopedPointer<QColorDialog> m_dialog;
};

QT_END_NAMESPACE

#endif // QWIDGETPLATFORMCOLORDIALOG_P_H
