# Generated from scenegraph.pro.

#####################################################################
## tst_scenegraph Test:
#####################################################################

# Collect test data
file(GLOB_RECURSE test_data_glob
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_scenegraph
    SOURCES
        tst_scenegraph.cpp
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::QuickPrivate
        Qt::QuickTestUtilsPrivate
    TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:scenegraph.pro:<TRUE>:
# OTHER_FILES = "data/render_OutOfFloatRange.qml" "data/simple.qml" "data/render_ImageFiltering.qml"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_scenegraph CONDITION ANDROID OR IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\":/data\\\"
)

qt_internal_extend_target(tst_scenegraph CONDITION NOT ANDROID AND NOT IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\\"
)
