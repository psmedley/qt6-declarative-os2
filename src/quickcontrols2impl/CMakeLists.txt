#####################################################################
## QuickControls2Impl Module:
#####################################################################

qt_internal_add_qml_module(QuickControls2Impl
    URI "QtQuick.Controls.impl"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME QtQuickControls2ImplPlugin
    DEPENDENCIES
        QtQuick/auto
    PLUGIN_TARGET qtquickcontrols2implplugin
    SOURCES
        qquickanimatednode.cpp qquickanimatednode_p.h
        qquickattachedobject.cpp qquickattachedobject_p.h
        qquickchecklabel.cpp qquickchecklabel_p.h
        qquickclippedtext.cpp qquickclippedtext_p.h
        qquickcolor.cpp qquickcolor_p.h
        qquickcolorimage.cpp qquickcolorimage_p.h
        qquickiconimage.cpp qquickiconimage_p.h
        qquickiconimage_p_p.h
        qquickiconlabel.cpp qquickiconlabel_p.h
        qquickiconlabel_p_p.h
        qquickitemgroup.cpp qquickitemgroup_p.h
        qquickmnemoniclabel.cpp qquickmnemoniclabel_p.h
        qquickpaddedrectangle.cpp qquickpaddedrectangle_p.h
        qquickplaceholdertext.cpp qquickplaceholdertext_p.h
        qtquickcontrols2foreign_p.h
        qtquickcontrols2implglobal_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::QuickPrivate
        Qt::QuickTemplates2Private
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Quick
    OS2_SHORT_NAME Qt6QuiCI
)

qt_internal_extend_target(QuickControls2Impl CONDITION QT_FEATURE_quick_listview AND QT_FEATURE_quick_pathview
    SOURCES
        qquicktumblerview.cpp qquicktumblerview_p.h
)
