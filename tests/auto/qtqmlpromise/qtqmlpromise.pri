TEMPLATE = app
CONFIG += qmltestcase
CONFIG += qtpromise-qml
DEFINES += QT_DEPRECATED_WARNINGS

QTPROMISE_IMPORTPATH = $$shadowed($$PWD/../../../qml)
DEFINES += QTPROMISE_IMPORTPATH=\"\\\"$$QTPROMISE_IMPORTPATH\\\"\"

include(../../../qtpromise.pri)
