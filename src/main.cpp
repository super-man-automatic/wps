#include <QApplication>
#include <clocale>
#include "gui/mainwindow.h"

#ifdef ENABLE_IMAGE_FEATURES
#include <opencv2/core/utils/logger.hpp>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Set Windows console to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Suppress OpenCV INFO logs (only show warnings and errors)
#ifdef ENABLE_IMAGE_FEATURES
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
#endif

    // Force UTF-8 locale for std::string conversions
    std::setlocale(LC_ALL, ".UTF8");
    QApplication app(argc, argv);

    gui::MainWindow window;
    window.show();

    return app.exec();
}
