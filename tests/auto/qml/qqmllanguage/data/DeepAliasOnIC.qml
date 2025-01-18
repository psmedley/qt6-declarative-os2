import QtQml

QtObject {
    id: root
    objectName: "theRoot"

    component ObjectWithColor: QtObject {
        property string color
        property var varvar
    }

    property ObjectWithColor border: ObjectWithColor {
        id: border
        objectName: root.objectName
        color: root.trueBorderColor
        varvar: root.trueBorderVarvar
    }

    readonly property rect readonlyRect: Qt.rect(12, 13, 14, 15)

    property alias borderObjectName: border.objectName
    property alias borderColor: border.color
    property alias borderVarvar: border.varvar
    property alias readonlyRectX: root.readonlyRect.x

    property string trueBorderColor: "green"
    property var trueBorderVarvar: 1234
}
