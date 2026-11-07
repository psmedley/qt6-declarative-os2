TEMPLATE = lib
CONFIG += plugin qmltypes
QT += qml quick

QML_IMPORT_NAME = ImageProviderCore
QML_IMPORT_MAJOR_VERSION = 1
DESTDIR = ImageProviderCore
TARGET  = qmlimageproviderplugin
os2:TARGET_SHORT = qmlipp
os2:CONFIG += target_short_symlink

SOURCES += imageprovider.cpp

EXAMPLE_FILES = imageprovider-example.qml

target.path = $$[QT_INSTALL_EXAMPLES]/quick/imageprovider/ImageProviderCore
qml.files = ImageProviderCore/qmldir
qml.path = $$[QT_INSTALL_EXAMPLES]/quick/imageprovider/ImageProviderCore
INSTALLS = target qml

CONFIG += install_ok  # Do not cargo-cult this!
