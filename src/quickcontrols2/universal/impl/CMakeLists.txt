#####################################################################
## qtquickcontrols2universalstyleimplplugin Plugin:
#####################################################################

set(qml_files
    "CheckIndicator.qml"
    "RadioIndicator.qml"
    "SwitchIndicator.qml"
)

qt_internal_add_qml_module(qtquickcontrols2universalstyleimplplugin
    URI "QtQuick.Controls.Universal.impl"
    VERSION "${PROJECT_VERSION}"
    PAST_MAJOR_VERSIONS 2
    CLASS_NAME QtQuickControls2UniversalStyleImplPlugin
    DEPENDENCIES
        QtQuick/auto
    PLUGIN_TARGET qtquickcontrols2universalstyleimplplugin
    NO_PLUGIN_OPTIONAL
    SOURCES
        qquickuniversalbusyindicator.cpp qquickuniversalbusyindicator_p.h
        qquickuniversalfocusrectangle.cpp qquickuniversalfocusrectangle_p.h
        qquickuniversalprogressbar.cpp qquickuniversalprogressbar_p.h
    QML_FILES
        ${qml_files}
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::QmlPrivate
        Qt::QuickControls2ImplPrivate
        Qt::QuickPrivate
        Qt::QuickTemplates2Private
    OS2_SHORT_NAME QuCoUnSI
)
