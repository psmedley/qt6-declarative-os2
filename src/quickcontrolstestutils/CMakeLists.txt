qt_internal_add_module(QuickControlsTestUtilsPrivate
    CONFIG_MODULE_NAME quickcontrolstestutilsprivate
    STATIC
    INTERNAL_MODULE
    SOURCES
        controlstestutils.cpp
        controlstestutils_p.h
        dialogstestutils_p.h
        dialogstestutils.cpp
        qtest_quickcontrols_p.h
    DEFINES
        QT_BUILD_SHARED_QUICK_CONTROLS_TEST_UTILS_LIB
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Test
        Qt::Qml
        Qt::QmlPrivate
        Qt::Quick
        Qt::QuickControls2
        Qt::QuickDialogs2QuickImplPrivate
        Qt::QuickPrivate
        Qt::QuickTemplates2
        Qt::QuickTemplates2Private
        Qt::QuickTestUtilsPrivate
)
