/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#ifndef QQMLCONTEXT_H
#define QQMLCONTEXT_H

#include <QtCore/qurl.h>
#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qpair.h>
#include <QtQml/qjsvalue.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE


class QString;
class QQmlEngine;
class QQmlRefCount;
class QQmlContextPrivate;
class QQmlCompositeTypeData;
class QQmlContextData;

class Q_QML_EXPORT QQmlContext : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QQmlContext)

public:
    struct PropertyPair { QString name; QVariant value; };

    QQmlContext(QQmlEngine *parent, QObject *objParent = nullptr);
    QQmlContext(QQmlContext *parent, QObject *objParent = nullptr);
    ~QQmlContext() override;

    bool isValid() const;

    QQmlEngine *engine() const;
    QQmlContext *parentContext() const;

    QObject *contextObject() const;
    void setContextObject(QObject *);

    QVariant contextProperty(const QString &) const;
    void setContextProperty(const QString &, QObject *);
    void setContextProperty(const QString &, const QVariant &);
    void setContextProperties(const QList<PropertyPair> &properties);

    QString nameForObject(const QObject *) const;
    QObject *objectForName(const QString &) const;

    QUrl resolvedUrl(const QUrl &) const;

    void setBaseUrl(const QUrl &);
    QUrl baseUrl() const;

    QJSValue importedScript(const QString &name) const;

private:
    friend class QQmlEngine;
    friend class QQmlEnginePrivate;
    friend class QQmlExpression;
    friend class QQmlExpressionPrivate;
    friend class QQmlComponent;
    friend class QQmlComponentPrivate;
    friend class QQmlScriptPrivate;
    friend class QQmlContextData;
    QQmlContext(QQmlContextPrivate &dd, QObject *parent = nullptr);
    QQmlContext(QQmlEngine *, bool);
    Q_DISABLE_COPY(QQmlContext)
};
QT_END_NAMESPACE

#endif // QQMLCONTEXT_H
