# Generated from multipointtoucharea_interop.pro.

#####################################################################
## tst_multipointtoucharea_interop Test:
#####################################################################

# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_multipointtoucharea_interop
    SOURCES
        tst_multipointtoucharea_interop.cpp
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::QuickPrivate
        Qt::QuickTestUtilsPrivate
    TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:multipointtoucharea_interop.pro:<TRUE>:
# OTHER_FILES = "data/pinchDragMPTA.qml"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_multipointtoucharea_interop CONDITION ANDROID OR IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\":/data\\\"
)

qt_internal_extend_target(tst_multipointtoucharea_interop CONDITION NOT ANDROID AND NOT IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\\"
)
