#include <QApplication>
<<<<<<< HEAD
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

=======
#include "gui/mainwindow.h"

int main(int argc, char* argv[]) {
>>>>>>> 5475e174cf498978befe4c9d3f475f4a1cc6c9bd
    QApplication app(argc, argv);

    gui::MainWindow window;
    window.show();

    return app.exec();
}
