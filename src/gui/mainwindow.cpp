#include "mainwindow.h"
#include "editor_widget.h"
#include "ai_sidebar.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>

namespace gui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("SmartDoc AI Editor");
    setWindowIcon(QIcon(":/resources/icon.png"));
    setGeometry(100, 100, 1200, 800);

    setupUI();
    setupMenuBar();
    setupConnections();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    // 创建中心widget
    auto central = new QWidget(this);
    auto layout = new QHBoxLayout(central);

    // 编辑器
    editor_ = std::make_unique<EditorWidget>();
    layout->addWidget(editor_.get(), 3);

    // AI侧边栏
    ai_sidebar_ = std::make_unique<AISidebar>();
    layout->addWidget(ai_sidebar_.get(), 1);

    layout->setSpacing(10);
    central->setLayout(layout);
    setCentralWidget(central);
}

void MainWindow::setupMenuBar() {
    auto file_menu = menuBar()->addMenu(tr("&File"));

    auto new_action = file_menu->addAction(tr("&New"));
    connect(new_action, &QAction::triggered, this, &MainWindow::onNewDocument);

    auto open_action = file_menu->addAction(tr("&Open"));
    connect(open_action, &QAction::triggered, this, &MainWindow::onOpenDocument);

    auto save_action = file_menu->addAction(tr("&Save"));
    connect(save_action, &QAction::triggered, this, &MainWindow::onSaveDocument);

    file_menu->addSeparator();

    auto exit_action = file_menu->addAction(tr("E&xit"));
    connect(exit_action, &QAction::triggered, this, &QWidget::close);

    auto help_menu = menuBar()->addMenu(tr("&Help"));
    auto about_action = help_menu->addAction(tr("&About"));
    connect(about_action, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupConnections() {
    // TODO: 连接编辑器和AI侧边栏的信号
}

void MainWindow::onNewDocument() {
    if (editor_) {
        editor_->clear();
    }
}

void MainWindow::onOpenDocument() {
    QString file_path = QFileDialog::getOpenFileName(
        this, tr("Open File"), "",
        tr("Markdown Files (*.md);;Text Files (*.txt);;All Files (*)"));

    if (!file_path.isEmpty() && editor_) {
        editor_->openFile(file_path.toStdString());
    }
}

void MainWindow::onSaveDocument() {
    if (editor_) {
        editor_->saveFile();
    }
}

void MainWindow::onAbout() {
    QMessageBox::about(this, tr("About SmartDoc"),
        tr("SmartDoc AI Editor v0.1.0\n\n"
           "A smart document editor with AI assistance.\n"
           "Copyright 2024"));
}

} // namespace gui
