# Generated from qquicksystempalette.pro.

#####################################################################
## tst_qquicksystempalette Test:
#####################################################################

# Collect test data
file(GLOB_RECURSE test_data_glob
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_qquicksystempalette
    SOURCES
        tst_qquicksystempalette.cpp
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

qt_internal_extend_target(tst_qquicksystempalette CONDITION TARGET Qt::Widgets
    PUBLIC_LIBRARIES
        Qt::Widgets
)

qt_internal_extend_target(tst_qquicksystempalette CONDITION ANDROID OR IOS
        DEFINES
    QT_QMLTEST_DATADIR=\\\":/data\\\"
)

qt_internal_extend_target(tst_qquicksystempalette CONDITION NOT ANDROID AND NOT IOS
        DEFINES
    QT_QMLTEST_DATADIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\\"
)
