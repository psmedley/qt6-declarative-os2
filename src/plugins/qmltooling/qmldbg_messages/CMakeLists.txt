# Generated from qmldbg_messages.pro.

#####################################################################
## QDebugMessageServiceFactory Plugin:
#####################################################################

qt_internal_add_plugin(QDebugMessageServiceFactoryPlugin
    OUTPUT_NAME qmldbg_messages
    CLASS_NAME QDebugMessageServiceFactory
    PLUGIN_TYPE qmltooling
    SOURCES
        qdebugmessageservice.cpp qdebugmessageservice.h
        qdebugmessageservicefactory.cpp qdebugmessageservicefactory.h
    LIBRARIES
        Qt::Core
        Qt::PacketProtocolPrivate
        Qt::QmlPrivate
    OS2_SHORT_NAME qmldbgme
)

#### Keys ignored in scope 1:.:.:qmldbg_messages.pro:<TRUE>:
# OTHER_FILES = "$$PWD/qdebugmessageservice.json"
