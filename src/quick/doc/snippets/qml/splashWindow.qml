/****************************************************************************
**
** Copyright (C) 2023 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

//! [entire]
import QtQuick

Window {
    id: mainWindow
    title: "Main Window"
    color: "#456"
    property real defaultSpacing: 10

    property Splash splash: Splash {
        onTimeout: mainWindow.show()
    }

    component Splash: Window {
        id: splash

        // a splash screen has no titlebar
        flags: Qt.SplashScreen
        // the transparent color lets background behind the image edges show through
        color: "transparent"
        modality: Qt.ApplicationModal // in case another application window is showing
        title: "Splash Window" // for the taskbar/dock, task switcher etc.
        visible: true

        // here we use the Screen attached property to center the splash window
        //! [screen-properties]
        x: (Screen.width - splashImage.width) / 2
        y: (Screen.height - splashImage.height) / 2
        //! [screen-properties]
        width: splashImage.width
        height: splashImage.height

        property int timeoutInterval: 2000
        signal timeout

        Image {
            id: splashImage
            source: "images/qt-logo.png"
        }

        TapHandler {
            onTapped: splash.timeout()
        }

        Timer {
            interval: splash.timeoutInterval; running: true; repeat: false
            onTriggered: {
                splash.visible = false
                splash.timeout()
            }
        }
    }
}
//! [entire]
