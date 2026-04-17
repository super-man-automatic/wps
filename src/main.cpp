#include <QApplication>
#include <clocale>
#include "gui/mainwindow.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Set Windows console to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Force UTF-8 locale for std::string conversions
    std::setlocale(LC_ALL, ".UTF8");

    QApplication app(argc, argv);

    gui::MainWindow window;
    window.show();

    return app.exec();
}
