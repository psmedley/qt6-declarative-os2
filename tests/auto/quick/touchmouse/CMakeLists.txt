# Generated from touchmouse.pro.

#####################################################################
## tst_touchmouse Test:
#####################################################################

# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_touchmouse
    SOURCES
        tst_touchmouse.cpp
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::QuickPrivate
        Qt::QuickTestUtilsPrivate
    TESTDATA ${test_data}
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_touchmouse CONDITION ANDROID OR IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\":/data\\\"
)

qt_internal_extend_target(tst_touchmouse CONDITION NOT ANDROID AND NOT IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\\"
)
