# Generated from qqmlxmlhttprequest.pro.

#####################################################################
## tst_qqmlxmlhttprequest Test:
#####################################################################

# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_qqmlxmlhttprequest
    SOURCES
        tst_qqmlxmlhttprequest.cpp
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::Network
        Qt::QmlPrivate
        Qt::QuickTestUtilsPrivate
    TESTDATA ${test_data}
)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(tst_qqmlxmlhttprequest)
endif()

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qqmlxmlhttprequest CONDITION ANDROID OR IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\":/data\\\"
)

qt_internal_extend_target(tst_qqmlxmlhttprequest CONDITION NOT ANDROID AND NOT IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\\"
)
