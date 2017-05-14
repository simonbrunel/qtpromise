TEMPLATE = subdirs
SUBDIRS = \
    src \
    tests

tests.depends = src

OTHER_FILES = \
    package/features/*.prf \
    qtpromise.pri
