# Generated from qmldbg_profiler.pro.

#####################################################################
## QQmlProfilerServiceFactory Plugin:
#####################################################################

qt_internal_add_plugin(QQmlProfilerServiceFactoryPlugin
    OUTPUT_NAME qmldbg_profiler
    CLASS_NAME QQmlProfilerServiceFactory
    PLUGIN_TYPE qmltooling
    SOURCES
        qqmlenginecontrolservice.cpp qqmlenginecontrolservice.h
        qqmlprofileradapter.cpp qqmlprofileradapter.h
        qqmlprofilerservice.cpp qqmlprofilerservice.h
        qqmlprofilerservicefactory.cpp qqmlprofilerservicefactory.h
        qv4profileradapter.cpp qv4profileradapter.h
    LIBRARIES
        Qt::CorePrivate
        Qt::PacketProtocolPrivate
        Qt::QmlPrivate
    OS2_SHORT_NAME qmldbgpr
)

#### Keys ignored in scope 1:.:.:qmldbg_profiler.pro:<TRUE>:
# OTHER_FILES = "$$PWD/qqmlprofilerservice.json"
