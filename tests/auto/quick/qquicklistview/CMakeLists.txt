# Generated from qquicklistview.pro.

#####################################################################
## tst_qquicklistview Test:
#####################################################################

# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_qquicklistview
    SOURCES
        incrementalmodel.cpp incrementalmodel.h
        proxytestinnermodel.cpp proxytestinnermodel.h
        randomsortmodel.cpp randomsortmodel.h
        reusemodel.h
        tst_qquicklistview.cpp
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::QmlModelsPrivate
        Qt::QmlPrivate
        Qt::QuickPrivate
        Qt::QuickTest
        Qt::QuickTestUtilsPrivate
    TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:qquicklistview.pro:<TRUE>:
# DISTFILES = "data/*"
# testcase.timeout = "900"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qquicklistview CONDITION ANDROID OR IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\":/data\\\"
)

qt_internal_extend_target(tst_qquicklistview CONDITION NOT ANDROID AND NOT IOS
    DEFINES
        QT_QMLTEST_DATADIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\\"
)
