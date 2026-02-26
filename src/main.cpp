#include "MainWindow.h"

#include <QApplication>
#include <QLoggingCategory>

#ifdef LE_DEBUG
#include <QLoggingCategory>
#endif

int main(int argc, char* argv[])
{
    // Enable High-DPI scaling (Qt6 does this by default, but explicit is clean)
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough
    );

    QApplication app(argc, argv);
    app.setApplicationName("LunateEpsilon");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("LunateEpsilon");

#ifdef LE_DEBUG
    QLoggingCategory::setFilterRules(
        "le.converter.debug=true\n"
        "le.theme.debug=true\n"
        "le.window.debug=true\n"
        "le.thread.debug=true\n"
    );
#else
    QLoggingCategory::setFilterRules(
        "le.*.debug=false\n"
    );
#endif

    LE::MainWindow window;
    window.show();

    return app.exec();
}