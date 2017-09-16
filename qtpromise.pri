INCLUDEPATH += $$PWD/include $$PWD/src
DEPENDPATH += $$PWD/include $$PWD/src
CONFIG += c++11

qtpromise-qml {
    QML_IMPORT_PATH += $$shadowed($$PWD)/qml

    # To avoid carrying an extra library dependency, the QJSPromise definition is
    # embedded in the QML plugin, so we need to link against the plugin itself.
    LIBS += -L$$shadowed($$PWD)/qml/QtPromise -l$$qtLibraryTarget(qtpromiseplugin)
}
