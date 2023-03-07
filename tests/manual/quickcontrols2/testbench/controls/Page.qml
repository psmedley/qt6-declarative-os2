// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

QtObject {
    property var supportedStates: [
        [],
    ]

    property Component component: Page {
        width: 100
        height: 100

        Label {
            text: "Page"
            anchors.centerIn: parent
        }
    }
}
