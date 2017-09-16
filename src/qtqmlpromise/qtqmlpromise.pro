TEMPLATE = aux
CONFIG += c++11 force_qt thread warn_on
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QTQMLPROMISE_LIBRARY
QT += qml

include($$PWD/../../qtpromise.pri)
include($$PWD/qtqmlpromise.pri)
