/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Labs Platform module of the Qt Toolkit.
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

#include "qquicklabsplatformstandardpaths_p.h"

#include <QtQml/qqmlengine.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype StandardPaths
    \inherits QtObject
//!     \instantiates QQuickLabsPlatformStandardPaths
    \inqmlmodule Qt.labs.platform
    \since 5.8
    \brief Provides access to the standard system paths.

    The StandardPaths singleton type provides methods for querying the standard
    system paths. The standard paths are mostly useful in conjunction with the
    FileDialog and FolderDialog types.

    \qml
    FileDialog {
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
    }
    \endqml

    \labs

    \sa FileDialog, FolderDialog, QStandardPaths
*/

static QList<QUrl> toUrlList(const QStringList &paths)
{
    QList<QUrl> urls;
    urls.reserve(paths.size());
    for (const QString &path : paths)
        urls += QUrl::fromLocalFile(path);
    return urls;
}

QQuickLabsPlatformStandardPaths::QQuickLabsPlatformStandardPaths(QObject *parent)
    : QObject(parent)
{
}

QObject *QQuickLabsPlatformStandardPaths::create(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine);
    return new QQuickLabsPlatformStandardPaths(engine);
}

/*!
    \qmlmethod string Qt.labs.platform::StandardPaths::displayName(StandardLocation type)

    \include standardpath/functiondocs.qdocinc displayName

    \sa QStandardPaths::displayName()
*/
QString QQuickLabsPlatformStandardPaths::displayName(QStandardPaths::StandardLocation type)
{
    return QStandardPaths::displayName(type);
}

/*!
    \qmlmethod url Qt.labs.platform::StandardPaths::findExecutable(string executableName, list<string> paths)

    \include standardpath/functiondocs.qdocinc findExecutable

    \sa QStandardPaths::findExecutable()
*/
QUrl QQuickLabsPlatformStandardPaths::findExecutable(const QString &executableName, const QStringList &paths)
{
    return QUrl::fromLocalFile(QStandardPaths::findExecutable(executableName, paths));
}

/*!
    \qmlmethod url Qt.labs.platform::StandardPaths::locate(StandardLocation type, string fileName, LocateOptions options)

    \include standardpath/functiondocs.qdocinc locate

    \sa QStandardPaths::locate()
*/
QUrl QQuickLabsPlatformStandardPaths::locate(QStandardPaths::StandardLocation type, const QString &fileName, QStandardPaths::LocateOptions options)
{
    return QUrl::fromLocalFile(QStandardPaths::locate(type, fileName, options));
}

/*!
    \qmlmethod list<url> Qt.labs.platform::StandardPaths::locateAll(StandardLocation type, string fileName, LocateOptions options)

    \include standardpath/functiondocs.qdocinc locateAll

    \sa QStandardPaths::locateAll()
*/
QList<QUrl> QQuickLabsPlatformStandardPaths::locateAll(QStandardPaths::StandardLocation type, const QString &fileName, QStandardPaths::LocateOptions options)
{
    return toUrlList(QStandardPaths::locateAll(type, fileName, options));
}

/*!
    \qmlmethod void Qt.labs.platform::StandardPaths::setTestModeEnabled(bool testMode)

    \include standardpath/functiondocs.qdocinc setTestModeEnabled

    \sa QStandardPaths::setTestModeEnabled()
*/
void QQuickLabsPlatformStandardPaths::setTestModeEnabled(bool testMode)
{
    QStandardPaths::setTestModeEnabled(testMode);
}

/*!
    \qmlmethod list<url> Qt.labs.platform::StandardPaths::standardLocations(StandardLocation type)

    \include standardpath/functiondocs.qdocinc standardLocations

    \sa QStandardPaths::standardLocations()
*/
QList<QUrl> QQuickLabsPlatformStandardPaths::standardLocations(QStandardPaths::StandardLocation type)
{
    return toUrlList(QStandardPaths::standardLocations(type));
}

/*!
    \qmlmethod url Qt.labs.platform::StandardPaths::writableLocation(StandardLocation type)

    \include standardpath/functiondocs.qdocinc writableLocation

    \sa QStandardPaths::writableLocation()
*/
QUrl QQuickLabsPlatformStandardPaths::writableLocation(QStandardPaths::StandardLocation type)
{
    return QUrl::fromLocalFile(QStandardPaths::writableLocation(type));
}

QT_END_NAMESPACE

#include "moc_qquicklabsplatformstandardpaths_p.cpp"
