#include "editor_widget.h"
#include "../core/document.h"
#include <QFont>
#include <QImage>
#include <QTextCursor>
#include <QTextImageFormat>
#include <QTextDocument>
#include <QUrl>
#include <QDateTime>
#include <fstream>
#include <sstream>

namespace gui {

EditorWidget::EditorWidget(QWidget* parent)
    : QTextEdit(parent),
      document_(std::make_shared<smartdoc::Document>()) {

    // 设置字体
    QFont font("Monaco", 12);
    font.setFixedPitch(true);
    setFont(font);

    // 连接信号
    connect(this, &QTextEdit::textChanged,
            this, &EditorWidget::onTextChanged);
    connect(this, &QTextEdit::selectionChanged,
            this, &EditorWidget::onSelectionChanged);

    setPlaceholderText("开始编写文档...\n使用侧边栏的AI功能辅助写作");
    
    // 启用撤销/重做功能
    setUndoRedoEnabled(true);
}

EditorWidget::~EditorWidget() {
}

void EditorWidget::onTextChanged() {
    std::string text = toPlainText().toStdString();
    document_->loadFromString(text);
    emit contentChanged(QString::fromStdString(text));
}

void EditorWidget::onSelectionChanged() {
    QTextCursor cursor = textCursor();
    QString selected = cursor.selectedText();
    emit selectionChanged(selected);
}

void EditorWidget::openFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) return;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    setPlainText(QString::fromStdString(content));
    document_->loadFromString(content);
    current_file_path_ = path;
}

void EditorWidget::saveFile() {
    if (current_file_path_.empty()) return;
    document_->saveFile(current_file_path_);
}

std::string EditorWidget::getText() const {
    return toPlainText().toStdString();
}

void EditorWidget::setText(const std::string& text) {
    setPlainText(QString::fromStdString(text));
}

void EditorWidget::insertImage(const QString& imagePath) {
    QImage image(imagePath);
    if (image.isNull()) {
        return;
    }

    // 缩放到合适大小
    if (image.width() > 600) {
        image = image.scaledToWidth(600, Qt::SmoothTransformation);
    }

    // Wrap in edit block for undo support
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    // 1. 把图片加入文档资源（关键！这样就不会显示占位符）
    QTextDocument* doc = document();
    QString imageName = "img_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    QUrl imageUrl(imageName);

    doc->addResource(QTextDocument::ImageResource, imageUrl, QVariant(image));

    // 2. 插入图片（可自定义显示大小）
    QTextImageFormat imageFormat;
    imageFormat.setName(imageName);  // 直接用名称，不要用 URL.toString()
    imageFormat.setWidth(image.width());
    imageFormat.setHeight(image.height());

    cursor.insertImage(imageFormat);

    // 换行
    cursor.insertText("\n");
    
    cursor.endEditBlock();
    setTextCursor(cursor);
}

} // namespace gui
