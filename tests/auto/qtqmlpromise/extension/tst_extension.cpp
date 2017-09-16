#include <QtQuickTest/quicktest.h>

static void initialize()
{
    qputenv("QML2_IMPORT_PATH", QTPROMISE_IMPORTPATH);
}

Q_COREAPP_STARTUP_FUNCTION(initialize)

QUICK_TEST_MAIN(extension)
