# Generated from qjsmanagedvalue.pro.

#####################################################################
## tst_qjsmanagedvalue Test:
#####################################################################

qt_internal_add_test(tst_qjsmanagedvalue
    SOURCES
        tst_qjsmanagedvalue.cpp tst_qjsmanagedvalue.h
    PUBLIC_LIBRARIES
        Qt::Qml
        Qt::QmlPrivate
    TESTDATA "dummy_imports.qml"
)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(tst_qjsmanagedvalue)
endif()

## Scopes:
#####################################################################
