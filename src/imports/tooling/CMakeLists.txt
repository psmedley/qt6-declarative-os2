set(qml_files
    "Component.qml"
    "Enum.qml"
    "Member.qml"
    "Method.qml"
    "Module.qml"
    "Parameter.qml"
    "Property.qml"
    "Signal.qml"
)

set_source_files_properties(${qml_files} PROPERTIES
    QT_QML_SOURCE_VERSIONS "1.2;6.0"
)

qt_internal_add_qml_module(quicktooling
    URI "QtQuick.tooling"
    VERSION "${PROJECT_VERSION}"
    NO_PLUGIN_OPTIONAL
    PLUGIN_TARGET quicktooling
    PAST_MAJOR_VERSIONS 1
    QML_FILES ${qml_files}
    OS2_SHORT_NAME QuickToo
)
