# Generated from qmldbg_nativedebugger.pro.

#####################################################################
## QQmlNativeDebugServiceFactory Plugin:
#####################################################################

qt_internal_add_plugin(QQmlNativeDebugServiceFactoryPlugin
    OUTPUT_NAME qmldbg_nativedebugger
    CLASS_NAME QQmlNativeDebugServiceFactory
    PLUGIN_TYPE qmltooling
    SOURCES
        qqmlnativedebugservice.cpp qqmlnativedebugservice.h
        qqmlnativedebugservicefactory.cpp qqmlnativedebugservicefactory.h
    LIBRARIES
        Qt::Core
        Qt::PacketProtocolPrivate
        Qt::QmlPrivate
    OS2_SHORT_NAME qmldbgnd
)

#### Keys ignored in scope 1:.:.:qmldbg_nativedebugger.pro:<TRUE>:
# OTHER_FILES = "$$PWD/qqmlnativedebugservice.json"
