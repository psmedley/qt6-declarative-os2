# Generated from qmldbg_tcp.pro.

#####################################################################
## QTcpServerConnectionFactory Plugin:
#####################################################################

qt_internal_add_plugin(QTcpServerConnectionFactoryPlugin
    OUTPUT_NAME qmldbg_tcp
    CLASS_NAME QTcpServerConnectionFactory
    PLUGIN_TYPE qmltooling
    SOURCES
        qtcpserverconnection.cpp
        qtcpserverconnectionfactory.h
    LIBRARIES
        Qt::Network
        Qt::QmlPrivate
    OS2_SHORT_NAME qmldbgtc
)

#### Keys ignored in scope 1:.:.:qmldbg_tcp.pro:<TRUE>:
# OTHER_FILES = "$$PWD/qtcpserverconnection.json"
