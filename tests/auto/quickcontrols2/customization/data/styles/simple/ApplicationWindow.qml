// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T

T.ApplicationWindow {
    id: control
    objectName: "applicationwindow-simple"

    minimumWidth: background.implicitWidth
    minimumHeight: background.implicitHeight

    background: Rectangle {
        objectName: "applicationwindow-background-simple"
        implicitWidth: 20
        implicitHeight: 20
    }
}
