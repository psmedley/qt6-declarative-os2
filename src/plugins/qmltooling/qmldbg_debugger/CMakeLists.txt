# Generated from qmldbg_debugger.pro.

#####################################################################
## QQmlDebuggerServiceFactory Plugin:
#####################################################################

qt_internal_add_plugin(QQmlDebuggerServiceFactoryPlugin
    OUTPUT_NAME qmldbg_debugger
    CLASS_NAME QQmlDebuggerServiceFactory
    PLUGIN_TYPE qmltooling
    SOURCES
        qqmldebuggerservicefactory.cpp qqmldebuggerservicefactory.h
        qqmlenginedebugservice.cpp qqmlenginedebugservice.h
        qqmlwatcher.cpp qqmlwatcher.h
        qv4datacollector.cpp qv4datacollector.h
        qv4debugger.cpp qv4debugger.h
        qv4debuggeragent.cpp qv4debuggeragent.h
        qv4debugjob.cpp qv4debugjob.h
        qv4debugservice.cpp qv4debugservice.h
    LIBRARIES
        Qt::CorePrivate
        Qt::PacketProtocolPrivate
        Qt::QmlPrivate
    OS2_SHORT_NAME qmldbgde
)

#### Keys ignored in scope 1:.:.:qmldbg_debugger.pro:<TRUE>:
# OTHER_FILES = "$$PWD/qqmldebuggerservice.json"
