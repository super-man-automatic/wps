#include "editor_widget.h"
#include "../core/document.h"
#include <QFont>
#include <fstream>
#include <sstream>

namespace gui {

EditorWidget::EditorWidget(QWidget* parent)
    : QPlainTextEdit(parent),
      document_(std::make_shared<smartdoc::Document>()) {

    // 设置字体
    QFont font("Monaco", 12);
    font.setFixedPitch(true);
    setFont(font);

    // 连接信号
    connect(this, &QPlainTextEdit::textChanged,
            this, &EditorWidget::onTextChanged);
    connect(this, &QPlainTextEdit::selectionChanged,
            this, &EditorWidget::onSelectionChanged);

    setPlaceholderText("开始编写文档...\n使用侧边栏的AI功能辅助写作");
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

} // namespace gui
