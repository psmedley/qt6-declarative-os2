qt_internal_add_qml_module(QmlCore
    URI "QtCore"
    VERSION "${PROJECT_VERSION}"
    DESIGNER_SUPPORTED
    PLUGIN_TARGET qtqmlcoreplugin
    CLASS_NAME QtQmlCorePlugin
    DEPENDENCIES
        QtQml
    SOURCES
        qqmlstandardpaths_p.h
        qqmlstandardpaths.cpp
        qqmlcoreglobal_p.h
    DEFINES
        QT_BUILD_QML_CORE_LIB
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Qml
    OS2_SHORT_NAME QtQmlcoe
)

qt_internal_add_docs(QmlCore
    doc/qtqmlcore.qdocconf
)
