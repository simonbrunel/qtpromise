TEMPLATE = lib
CONFIG += plugin exceptions
QT += qml

IMPORT_VERSION = 1.0
DEFINES += QTQMLPROMISE_LIBRARY

TARGET = $$qtLibraryTarget(qtpromiseplugin)
DESTDIR = $$shadowed($$PWD/../../qml/QtPromise)

include(../qtqmlpromise/qtqmlpromise.pri)
include(../../qtpromise.pri)

SOURCES += \
    $$PWD/plugin.cpp

QMLFILES += \
    $$PWD/plugins.qmltypes \
    $$PWD/qmldir

RESOURCES += $$PWD/imports.qrc

qmlfiles.files = $$QMLFILES
qmlfiles.path = $$DESTDIR
COPIES += qmlfiles
