# Generated from qmldbg_local.pro.

#####################################################################
## QLocalClientConnectionFactory Plugin:
#####################################################################

qt_internal_add_plugin(QLocalClientConnectionFactoryPlugin
    OUTPUT_NAME qmldbg_local
    CLASS_NAME QLocalClientConnectionFactory
    PLUGIN_TYPE qmltooling
    SOURCES
        qlocalclientconnection.cpp
        qlocalclientconnectionfactory.h
    LIBRARIES
        Qt::QmlPrivate
    OS2_SHORT_NAME qmldbglo
)

#### Keys ignored in scope 1:.:.:qmldbg_local.pro:<TRUE>:
# OTHER_FILES = "$$PWD/qlocalclientconnection.json"
