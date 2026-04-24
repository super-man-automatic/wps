#include "mainwindow.h"
#include "editor_widget.h"
#include "ai_sidebar.h"
#ifdef ENABLE_IMAGE_FEATURES
#include "image_editor_dialog.h"
#endif
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextCursor>
#include <QTextImageFormat>
#include <QTextDocument>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QFont>
#include <QRegularExpression>
#include <QStatusBar>
#include <QShortcut>
#include <QApplication>
#ifdef ENABLE_IMAGE_FEATURES
#include <QImageReader>
#include <QBuffer>
#endif

namespace gui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("SmartDoc AI Editor");
    setWindowIcon(QIcon(":/resources/icon.png"));
    setGeometry(100, 100, 1200, 800);

    setupUI();
    setupMenuBar();
    setupStatusBar();
    setupConnections();
    
    // Setup direct shortcuts for font size (work even when editor has focus)
    auto font_inc_shortcut = new QShortcut(QKeySequence("Ctrl+="), this);
    font_inc_shortcut->setContext(Qt::ApplicationShortcut);
    connect(font_inc_shortcut, &QShortcut::activated, this, &MainWindow::onFontSizeIncrease);
    
    auto font_dec_shortcut = new QShortcut(QKeySequence("Ctrl+-"), this);
    font_dec_shortcut->setContext(Qt::ApplicationShortcut);
    connect(font_dec_shortcut, &QShortcut::activated, this, &MainWindow::onFontSizeDecrease);
    
    updateStatusBar();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    // 创建中心widget
    auto central = new QWidget(this);
    auto layout = new QHBoxLayout(central);

    // 编辑器
    editor_ = std::make_unique<EditorWidget>();
    editor_->installEventFilter(this); // Install event filter to catch Ctrl+C
    layout->addWidget(editor_.get(), 3);

    // AI侧边栏
    ai_sidebar_ = std::make_unique<AISidebar>();
    layout->addWidget(ai_sidebar_.get(), 1);

    layout->setSpacing(10);
    central->setLayout(layout);
    setCentralWidget(central);
}

void MainWindow::setupMenuBar() {
    // File menu
    auto file_menu = menuBar()->addMenu(tr("&File"));

    auto new_action = file_menu->addAction(tr("&New"));
    new_action->setShortcut(QKeySequence::New);
    connect(new_action, &QAction::triggered, this, &MainWindow::onNewDocument);

    auto open_action = file_menu->addAction(tr("&Open..."));
    open_action->setShortcut(QKeySequence::Open);
    connect(open_action, &QAction::triggered, this, &MainWindow::onOpenDocument);

    auto save_action = file_menu->addAction(tr("&Save"));
    save_action->setShortcut(QKeySequence::Save);
    connect(save_action, &QAction::triggered, this, &MainWindow::onSaveDocument);

    auto save_as_action = file_menu->addAction(tr("Save &As..."));
    save_as_action->setShortcut(QKeySequence::SaveAs);
    connect(save_as_action, &QAction::triggered, this, &MainWindow::onSaveAsDocument);

    file_menu->addSeparator();

    auto clear_action = file_menu->addAction(tr("&Clear"));
    connect(clear_action, &QAction::triggered, this, &MainWindow::onClearDocument);

    file_menu->addSeparator();

    auto exit_action = file_menu->addAction(tr("E&xit"));
    exit_action->setShortcut(QKeySequence::Quit);
    connect(exit_action, &QAction::triggered, this, &QWidget::close);

    // Edit menu - Undo/Redo support
    auto edit_menu = menuBar()->addMenu(tr("&Edit"));
    
    auto undo_action = edit_menu->addAction(tr("&Undo"));
    undo_action->setShortcut(QKeySequence::Undo);
    connect(undo_action, &QAction::triggered, this, [this]() {
        if (editor_) editor_->undo();
    });
    
    auto redo_action = edit_menu->addAction(tr("&Redo"));
    redo_action->setShortcut(QKeySequence::Redo);
    connect(redo_action, &QAction::triggered, this, [this]() {
        if (editor_) editor_->redo();
    });
    
    edit_menu->addSeparator();
    
    auto cut_action = edit_menu->addAction(tr("Cu&t"));
    cut_action->setShortcut(QKeySequence::Cut);
    connect(cut_action, &QAction::triggered, this, [this]() {
        if (editor_) editor_->cut();
    });
    
    auto copy_action = edit_menu->addAction(tr("&Copy"));
    copy_action->setShortcut(QKeySequence::Copy);
    connect(copy_action, &QAction::triggered, this, [this]() {
        if (editor_) editor_->copy();
    });
    
    auto paste_action = edit_menu->addAction(tr("&Paste"));
    paste_action->setShortcut(QKeySequence::Paste);
    connect(paste_action, &QAction::triggered, this, [this]() {
        if (editor_) editor_->paste();
    });

    // Format menu
    auto format_menu = menuBar()->addMenu(tr("F&ormat"));

    auto font_increase = format_menu->addAction(tr("Font &Size+ (Ctrl+>)"));
    font_increase->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Greater));
    connect(font_increase, &QAction::triggered, this, &MainWindow::onFontSizeIncrease);

    auto font_decrease = format_menu->addAction(tr("Font Size&- (Ctrl+<)"));
    font_decrease->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Less));
    connect(font_decrease, &QAction::triggered, this, &MainWindow::onFontSizeDecrease);

    // View menu
    auto view_menu = menuBar()->addMenu(tr("&View"));

    auto theme_action = view_menu->addAction(tr("&Toggle Dark/Light Mode"));
    theme_action->setShortcut(QKeySequence("Ctrl+T"));
    connect(theme_action, &QAction::triggered, this, &MainWindow::onToggleTheme);

#ifdef ENABLE_IMAGE_FEATURES
    // Image menu
    auto image_menu = menuBar()->addMenu(tr("&Image"));
    
    auto insert_image_action = image_menu->addAction(tr("&Insert Image from File..."));
    insert_image_action->setShortcut(QKeySequence("Ctrl+Shift+I"));
    connect(insert_image_action, &QAction::triggered, this, &MainWindow::onInsertImageFromFile);
    
    auto edit_image_action = image_menu->addAction(tr("&Edit Image..."));
    connect(edit_image_action, &QAction::triggered, this, &MainWindow::onEditImage);
    
    image_menu->addSeparator();
    
    auto resize_image_action = image_menu->addAction(tr("&Resize Image..."));
    connect(resize_image_action, &QAction::triggered, this, &MainWindow::onResizeImage);
    
    auto rotate_image_action = image_menu->addAction(tr("&Rotate Image..."));
    connect(rotate_image_action, &QAction::triggered, this, &MainWindow::onRotateImage);
    
    auto crop_image_action = image_menu->addAction(tr("&Crop Image..."));
    connect(crop_image_action, &QAction::triggered, this, &MainWindow::onCropImage);
    
    image_menu->addSeparator();
    
    auto grayscale_action = image_menu->addAction(tr("Apply &Grayscale"));
    connect(grayscale_action, &QAction::triggered, this, &MainWindow::onApplyGrayscale);
    
    auto blur_action = image_menu->addAction(tr("Apply &Blur"));
    connect(blur_action, &QAction::triggered, this, &MainWindow::onApplyBlur);
    
    auto sharpen_action = image_menu->addAction(tr("Apply &Sharpen"));
    connect(sharpen_action, &QAction::triggered, this, &MainWindow::onApplySharpen);
#endif

    // Help menu
    auto help_menu = menuBar()->addMenu(tr("&Help"));
    auto about_action = help_menu->addAction(tr("&About"));
    connect(about_action, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupStatusBar() {
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::setupConnections() {
    // Connect editor text changes to AI sidebar
    if (editor_ && ai_sidebar_) {
        connect(editor_.get(), &EditorWidget::contentChanged,
                ai_sidebar_.get(), &AISidebar::onTextChanged);
        connect(editor_.get(), &EditorWidget::selectionChanged,
                ai_sidebar_.get(), &AISidebar::onTextSelected);
        
        // Connect AI sidebar insert request to editor
        connect(ai_sidebar_.get(), &AISidebar::insertTextRequested,
                this, [this](const QString& text) {
            if (editor_) {
                QTextCursor cursor = editor_->textCursor();
                cursor.insertText(text);
            }
        });
        
        // Connect AI sidebar insert at position request to editor
        connect(ai_sidebar_.get(), &AISidebar::insertTextAtPosition,
                this, [this](const QString& text, int position) {
            if (editor_) {
                QTextCursor cursor = editor_->textCursor();
                cursor.setPosition(position);
                cursor.insertText(text);
            }
        });
        
        // Connect cursor position and text request from AI sidebar
        connect(ai_sidebar_.get(), &AISidebar::requestCursorPositionAndText,
                this, [this]() {
            if (editor_) {
                int pos = editor_->textCursor().position();
                QString text_before = editor_->toPlainText().left(pos);
                ai_sidebar_->onCursorPositionAndTextReceived(pos, text_before);
            }
        });
        
        // Update status bar when text changes
        connect(editor_.get(), &EditorWidget::contentChanged,
                this, &MainWindow::updateStatusBar);
    }
}

void MainWindow::onNewDocument() {
    current_file_path_.clear();
    if (editor_) {
        editor_->clear();
    }
    updateStatusBar();
}

void MainWindow::onOpenDocument() {
    QString file_path = QFileDialog::getOpenFileName(
        this, tr("Open File"), "",
        tr("Text Files (*.txt);;All Files (*)"));

    if (!file_path.isEmpty()) {
        loadFile(file_path);
    }
}

void MainWindow::onSaveDocument() {
    if (current_file_path_.isEmpty()) {
        onSaveAsDocument();
    } else {
        saveFile(current_file_path_);
    }
}

void MainWindow::onSaveAsDocument() {
    QString file_path = QFileDialog::getSaveFileName(
        this, tr("Save As"), "",
        tr("Text Files (*.txt);;All Files (*)"));

    if (!file_path.isEmpty()) {
        saveFile(file_path);
    }
}

void MainWindow::onClearDocument() {
    if (editor_) {
        editor_->clear();
    }
    updateStatusBar();
}

void MainWindow::onAbout() {
    QMessageBox::about(this, tr("About SmartDoc"),
        tr("SmartDoc AI Editor v0.2.0\n\n"
           "A smart document editor with AI assistance.\n"
           "Features: AI writing, file operations, dark/light mode.\n"
           "Copyright 2024"));
}

void MainWindow::onFontSizeIncrease() {
    if (!editor_) return;
    
    QTextCursor cursor = editor_->textCursor();
    if (!cursor.hasSelection()) {
        // No selection - show message in status bar
        statusBar()->showMessage(tr("Please select text to change font size"), 2000);
        return;
    }
    
    // Get current font size of selection
    QTextCharFormat format = cursor.charFormat();
    int current_size = format.fontPointSize();
    if (current_size <= 0) current_size = current_font_size_; // Use default if not set
    
    int new_size = current_size + 2;
    if (new_size > 32) new_size = 32;
    
    // Apply new font size to selection only
    format.setFontPointSize(new_size);
    cursor.mergeCharFormat(format);
    
    updateStatusBar();
}

void MainWindow::onFontSizeDecrease() {
    if (!editor_) return;
    
    QTextCursor cursor = editor_->textCursor();
    if (!cursor.hasSelection()) {
        // No selection - show message in status bar
        statusBar()->showMessage(tr("Please select text to change font size"), 2000);
        return;
    }
    
    // Get current font size of selection
    QTextCharFormat format = cursor.charFormat();
    int current_size = format.fontPointSize();
    if (current_size <= 0) current_size = current_font_size_; // Use default if not set
    
    int new_size = current_size - 2;
    if (new_size < 8) new_size = 8;
    
    // Apply new font size to selection only
    format.setFontPointSize(new_size);
    cursor.mergeCharFormat(format);
    
    updateStatusBar();
}

void MainWindow::onToggleTheme() {
    is_dark_theme_ = !is_dark_theme_;
    
    if (is_dark_theme_) {
        // Dark theme
        setStyleSheet(
            "QMainWindow { background-color: #2b2b2b; }"
            "QMenuBar { background-color: #3c3f41; color: #bbbbbb; }"
            "QMenuBar::item:selected { background-color: #4b6eaf; }"
            "QMenu { background-color: #3c3f41; color: #bbbbbb; }"
            "QMenu::item:selected { background-color: #4b6eaf; }"
            "QStatusBar { background-color: #3c3f41; color: #bbbbbb; }"
        );
    } else {
        // Light theme
        setStyleSheet("");
    }
}

void MainWindow::updateStatusBar() {
    QString file_name = current_file_path_.isEmpty() 
        ? tr("Untitled") 
        : QFileInfo(current_file_path_).fileName();
    
    int char_count = 0;
    int word_count = 0;
    if (editor_) {
        QString text = editor_->toPlainText();
        char_count = text.length();
        word_count = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).count();
    }
    
    QString message = tr("%1 | Font: %2pt | Characters: %3 | Words: %4")
        .arg(file_name)
        .arg(current_font_size_)
        .arg(char_count)
        .arg(word_count);
    
    statusBar()->showMessage(message);
}

void MainWindow::loadFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file: %1").arg(path));
        return;
    }
    
    QTextStream stream(&file);
    QString content = stream.readAll();
    file.close();
    
    if (editor_) {
        editor_->setPlainText(content);
    }
    
    current_file_path_ = path;
    setWindowTitle(tr("SmartDoc AI Editor - %1").arg(QFileInfo(path).fileName()));
    updateStatusBar();
}

void MainWindow::saveFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot save file: %1").arg(path));
        return;
    }
    
    QTextStream stream(&file);
    if (editor_) {
        stream << editor_->toPlainText();
    }
    file.close();
    
    current_file_path_ = path;
    setWindowTitle(tr("SmartDoc AI Editor - %1").arg(QFileInfo(path).fileName()));
    updateStatusBar();
    
    statusBar()->showMessage(tr("Saved to %1").arg(path), 2000);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    // Handle Ctrl+C to stop AI generation
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C) {
        if (ai_sidebar_) {
            ai_sidebar_->stopGeneration();
        }
        return; // Don't propagate to editor
    }
    
    #ifdef ENABLE_IMAGE_FEATURES
    // Handle Ctrl+Z for image undo first, then fall back to document undo
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Z) {
        if (editor_ && tryUndoImageEdit()) {
            return; // Image undo handled
        }
    }
#endif
    
    // Default handling for other keys
    QMainWindow::keyPressEvent(event);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    // Intercept Ctrl+C even when editor has focus
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_C) {
            // Only handle if it's a copy operation (not when text is selected)
            if (editor_ && !editor_->textCursor().hasSelection()) {
                if (ai_sidebar_) {
                    ai_sidebar_->stopGeneration();
                }
                return true; // Event handled, don't propagate
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

#ifdef ENABLE_IMAGE_FEATURES
// 纯 Qt 实现，完美显示图片，插入时无 OpenCV 依赖
void MainWindow::onInsertImageFromFile() {
    QString file_path = QFileDialog::getOpenFileName(
        this, tr("Insert Image"), "",
        tr("Images (*.png *.jpg *.jpeg *.bmp *.gif)"));

    if (file_path.isEmpty()) return;

    // 调用 EditorWidget 的 insertImage 方法
    if (editor_) {
        editor_->insertImage(file_path);
        statusBar()->showMessage("Image inserted", 2000);
    }
}

// Helper to find all images in selection - returns list of (position, image_name) pairs
static QList<QPair<int, QString>> findSelectedImages(QTextEdit* editor) {
    QList<QPair<int, QString>> results;
    QTextCursor cursor = editor->textCursor();
    
    if (!cursor.hasSelection()) {
        // No selection - check if cursor is on a single image
        int pos = cursor.position();
        QTextImageFormat fmt = cursor.charFormat().toImageFormat();
        if (!fmt.isValid() && pos > 0) {
            cursor.setPosition(pos - 1);
            fmt = cursor.charFormat().toImageFormat();
            if (fmt.isValid()) {
                pos = pos - 1;
            }
        }
        if (fmt.isValid()) {
            results.append(qMakePair(pos, fmt.name()));
        }
        return results;
    }
    
    // Has selection - find all images in the selected range
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    
    QTextCursor scan(editor->document());
    for (int pos = start; pos < end; ++pos) {
        scan.setPosition(pos);
        QTextImageFormat fmt = scan.charFormat().toImageFormat();
        if (fmt.isValid()) {
            results.append(qMakePair(pos, fmt.name()));
        }
    }
    
    return results;
}

void MainWindow::onEditImage() {
    if (!editor_) return;
    
    auto images = findSelectedImages(editor_.get());
    if (images.isEmpty()) {
        QMessageBox::information(this, tr("No Image"), tr("Please select an image first."));
        return;
    }
    
    // For simplicity, edit only the first selected image
    int pos = images.first().first;
    QString img_name = images.first().second;
    
    QTextCursor cursor(editor_->document());
    cursor.setPosition(pos);
    
    QImage image = editor_->document()->resource(QTextDocument::ImageResource, QUrl(img_name)).value<QImage>();
    if (image.isNull()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to get image"));
        return;
    }
    
    if (!image_editor_) {
        image_editor_ = std::make_unique<ImageEditorDialog>(this);
    }
    
    if (image_editor_->loadImage(image)) {
        if (image_editor_->exec() == QDialog::Accepted) {
            QImage result = image_editor_->getResultImage();
            if (!result.isNull()) {
                cursor.beginEditBlock();
                cursor.deleteChar();
                // Generate NEW unique name for edited image (enables undo)
                QString new_img_name = "img_" + QString::number(QDateTime::currentMSecsSinceEpoch());
                editor_->document()->addResource(QTextDocument::ImageResource, QUrl(new_img_name), QVariant(result));
                QTextImageFormat new_fmt;
                new_fmt.setName(new_img_name);
                new_fmt.setWidth(result.width());
                new_fmt.setHeight(result.height());
                new_fmt.setVerticalAlignment(QTextCharFormat::AlignMiddle);
                cursor.insertImage(new_fmt);
                cursor.endEditBlock();
                cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
                editor_->setTextCursor(cursor);
                statusBar()->showMessage(tr("Image edited successfully"), 2000);
            }
        }
    }
}

void MainWindow::onResizeImage() {
    onEditImage();  // Use same logic as edit
}

void MainWindow::onRotateImage() {
    onEditImage();  // Use same logic as edit
}

void MainWindow::onCropImage() {
    onEditImage();  // Use same logic as edit
}

void MainWindow::onApplyGrayscale() {
    if (!editor_) return;
    
    auto images = findSelectedImages(editor_.get());
    if (images.isEmpty()) {
        QMessageBox::information(this, tr("No Image"), tr("Please select one or more images first."));
        return;
    }
    
    if (!image_editor_) {
        image_editor_ = std::make_unique<ImageEditorDialog>(this);
    }
    
        int count = 0;
    for (const auto& img_info : images) {
        int pos = img_info.first;
        QString img_name = img_info.second;
        QTextCursor cursor(editor_->document());
        cursor.setPosition(pos);
        
        QImage image = editor_->document()->resource(QTextDocument::ImageResource, QUrl(img_name)).value<QImage>();
        if (image.isNull()) continue;
        
        if (image_editor_->loadImage(image)) {
            saveImageToHistory(img_name, image);
            image_editor_->applyGrayscale();
            QImage result = image_editor_->getResultImage();
            if (!result.isNull()) {
                cursor.beginEditBlock();
                // Delete old image
                cursor.deleteChar();
                // Generate NEW unique name for edited image (enables undo)
                QString new_img_name = "img_" + QString::number(QDateTime::currentMSecsSinceEpoch());
                editor_->document()->addResource(QTextDocument::ImageResource, QUrl(new_img_name), QVariant(result));
                QTextImageFormat new_fmt;
                new_fmt.setName(new_img_name);
                new_fmt.setWidth(result.width());
                new_fmt.setHeight(result.height());
                new_fmt.setVerticalAlignment(QTextCharFormat::AlignMiddle);
                cursor.insertImage(new_fmt);
                cursor.endEditBlock();
                cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
                editor_->setTextCursor(cursor);
                count++;
            }
        }
    }
    
    if (count > 0) {
        statusBar()->showMessage(tr("Grayscale applied to %1 image(s)").arg(count), 2000);
    }
}

void MainWindow::onApplyBlur() {
    if (!editor_) return;
    
    auto images = findSelectedImages(editor_.get());
    if (images.isEmpty()) {
        QMessageBox::information(this, tr("No Image"), tr("Please select one or more images first."));
        return;
    }
    
    if (!image_editor_) {
        image_editor_ = std::make_unique<ImageEditorDialog>(this);
    }
    
    int count = 0;
    for (const auto& img_info : images) {
        int pos = img_info.first;
        QString img_name = img_info.second;
        QTextCursor cursor(editor_->document());
        cursor.setPosition(pos);
        
        QImage image = editor_->document()->resource(QTextDocument::ImageResource, QUrl(img_name)).value<QImage>();
        if (image.isNull()) continue;
        
        if (image_editor_->loadImage(image)) {
            saveImageToHistory(img_name, image);
            image_editor_->applyBlur(5);
            QImage result = image_editor_->getResultImage();
            if (!result.isNull()) {
                cursor.beginEditBlock();
                cursor.deleteChar();
                QString new_img_name = "img_" + QString::number(QDateTime::currentMSecsSinceEpoch());
                editor_->document()->addResource(QTextDocument::ImageResource, QUrl(new_img_name), QVariant(result));
                QTextImageFormat new_fmt;
                new_fmt.setName(new_img_name);
                new_fmt.setWidth(result.width());
                new_fmt.setHeight(result.height());
                new_fmt.setVerticalAlignment(QTextCharFormat::AlignMiddle);
                cursor.insertImage(new_fmt);
                cursor.endEditBlock();
                cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
                editor_->setTextCursor(cursor);
                count++;
            }
        }
    }
    
    if (count > 0) {
        statusBar()->showMessage(tr("Blur applied to %1 image(s)").arg(count), 2000);
    }
}

void MainWindow::onApplySharpen() {
    if (!editor_) return;
    
    auto images = findSelectedImages(editor_.get());
    if (images.isEmpty()) {
        QMessageBox::information(this, tr("No Image"), tr("Please select one or more images first."));
        return;
    }
    
    if (!image_editor_) {
        image_editor_ = std::make_unique<ImageEditorDialog>(this);
    }
    
    int count = 0;
    for (const auto& img_info : images) {
        int pos = img_info.first;
        QString img_name = img_info.second;
        QTextCursor cursor(editor_->document());
        cursor.setPosition(pos);
        
        QImage image = editor_->document()->resource(QTextDocument::ImageResource, QUrl(img_name)).value<QImage>();
        if (image.isNull()) continue;
        
        if (image_editor_->loadImage(image)) {
            saveImageToHistory(img_name, image);
            image_editor_->applySharpen();
            QImage result = image_editor_->getResultImage();
            if (!result.isNull()) {
                cursor.beginEditBlock();
                cursor.deleteChar();
                QString new_img_name = "img_" + QString::number(QDateTime::currentMSecsSinceEpoch());
                editor_->document()->addResource(QTextDocument::ImageResource, QUrl(new_img_name), QVariant(result));
                QTextImageFormat new_fmt;
                new_fmt.setName(new_img_name);
                new_fmt.setWidth(result.width());
                new_fmt.setHeight(result.height());
                new_fmt.setVerticalAlignment(QTextCharFormat::AlignMiddle);
                cursor.insertImage(new_fmt);
                cursor.endEditBlock();
                cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
                editor_->setTextCursor(cursor);
                count++;
            }
        }
    }
    
    if (count > 0) {
        statusBar()->showMessage(tr("Sharpen applied to %1 image(s)").arg(count), 2000);
    }
}

void MainWindow::onInsertImage() {
    onInsertImageFromFile();
}

// Save image to history before editing (for undo support)
void MainWindow::saveImageToHistory(const QString& img_name, const QImage& image) {
    if (!image_history_.contains(img_name)) {
        image_history_[img_name] = QList<QImage>();
    }
    image_history_[img_name].append(image.copy());
    while (image_history_[img_name].size() > 10) {
        image_history_[img_name].removeFirst();
    }
}

// Try to undo image edit - returns true if handled
bool MainWindow::tryUndoImageEdit() {
    if (!editor_) return false;
    
    // Step 1: Check if current position has an image with history
    QTextCursor cursor = editor_->textCursor();
    QTextImageFormat img_fmt = cursor.charFormat().toImageFormat();
    
    if (!img_fmt.isValid()) {
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        img_fmt = cursor.charFormat().toImageFormat();
    }
    
    if (!img_fmt.isValid()) {
        qDebug() << "tryUndoImageEdit: No image at cursor";
        return false; // No image here
    }
    
    QString current_img_name = img_fmt.name();
    qDebug() << "tryUndoImageEdit: Current image:" << current_img_name;
    qDebug() << "tryUndoImageEdit: History keys:" << image_history_.keys();
    
    // Step 2: Check if we have history for this image name
    // (This handles the case where we reuse the same name)
    if (image_history_.contains(current_img_name) && !image_history_[current_img_name].isEmpty()) {
        qDebug() << "tryUndoImageEdit: Found direct history for" << current_img_name;
        QImage previous = image_history_[current_img_name].takeLast();
        
        // Undo document operation first
        editor_->undo();
        
        // Restore resource
        editor_->document()->addResource(QTextDocument::ImageResource, QUrl(current_img_name), QVariant(previous));
        editor_->document()->setModified(true);
        editor_->viewport()->update();
        
        qDebug() << "tryUndoImageEdit: Restored (direct match)";
        return true;
    }
    
    // Step 3: If no direct history, this might be a new-named image after edit
    // We need to undo and then check what image appears
    qDebug() << "tryUndoImageEdit: No direct history, performing undo to find old image";
    
    bool had_history = false;
    QString old_img_name;
    QImage previous_image;
    
    // Perform the undo
    editor_->undo();
    
    // Now check what image is at the cursor after undo
    QTextCursor after_cursor = editor_->textCursor();
    QTextImageFormat after_fmt = after_cursor.charFormat().toImageFormat();
    
    if (!after_fmt.isValid()) {
        after_cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        after_fmt = after_cursor.charFormat().toImageFormat();
    }
    
    if (after_fmt.isValid()) {
        old_img_name = after_fmt.name();
        qDebug() << "tryUndoImageEdit: After undo, found image:" << old_img_name;
        
        if (image_history_.contains(old_img_name) && !image_history_[old_img_name].isEmpty()) {
            previous_image = image_history_[old_img_name].takeLast();
            had_history = true;
            qDebug() << "tryUndoImageEdit: Found history for old image";
        } else {
            qDebug() << "tryUndoImageEdit: No history for old image";
        }
    } else {
        qDebug() << "tryUndoImageEdit: No image found after undo";
    }
    
    if (had_history) {
        // Restore the old image resource
        editor_->document()->addResource(QTextDocument::ImageResource, QUrl(old_img_name), QVariant(previous_image));
        editor_->document()->setModified(true);
        editor_->viewport()->update();
        qDebug() << "tryUndoImageEdit: Restored old image resource";
        return true;
    }
    
    // No image history found, undo was already performed
    qDebug() << "tryUndoImageEdit: No history found, returning true anyway";
    return true; // We handled the undo (even though no image was restored)
}
#endif

} // namespace gui
