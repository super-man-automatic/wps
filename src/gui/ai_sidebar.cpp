#include "ai_sidebar.h"
#include <QLabel>
#include <QGroupBox>
#include <QtConcurrent>
#include <QThread>

namespace gui {

AISidebar::AISidebar(QWidget* parent)
    : QWidget(parent) {
    // Initialize AI client with Xunfei backend
    ai_client_ = std::make_shared<ai::LLMClient>(ai::Backend::Xunfei);
    
    // Configure Xunfei API credentials
    ai_client_->setAppId("5d460b66");
    ai_client_->setApiKey("6f7d2da1a23bc29e761708ac7d53f022");
    ai_client_->setApiSecret("MTEwOTA3MmI2MThlZWM2YjFlMjYxMzM1");
    
    ai_writer_ = std::make_shared<ai::AIWriter>(ai_client_);

    setupUI();
}

AISidebar::~AISidebar() {
}

void AISidebar::setupUI() {
    auto main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(5, 5, 5, 5);
    main_layout->setSpacing(8);

    // 标题
    auto title = new QLabel(reinterpret_cast<const char*>(u8"AI助手"));
    title->setStyleSheet("font-size: 14px; font-weight: bold;");
    main_layout->addWidget(title);

    // AI功能按钮组
    auto func_group = new QGroupBox(reinterpret_cast<const char*>(u8"快速功能"), this);
    auto func_layout = new QVBoxLayout(func_group);

    continue_btn_ = new QPushButton(reinterpret_cast<const char*>(u8"📝 续写"));
    connect(continue_btn_, &QPushButton::clicked, this, &AISidebar::onContinueWriting);
    func_layout->addWidget(continue_btn_);

    polish_btn_ = new QPushButton(reinterpret_cast<const char*>(u8"✏️ 润色"));
    connect(polish_btn_, &QPushButton::clicked, this, &AISidebar::onPolish);
    func_layout->addWidget(polish_btn_);

    summarize_btn_ = new QPushButton(reinterpret_cast<const char*>(u8"📋 摘要"));
    connect(summarize_btn_, &QPushButton::clicked, this, &AISidebar::onSummarize);
    func_layout->addWidget(summarize_btn_);

    main_layout->addWidget(func_group);

    // 聊天区域
    auto chat_group = new QGroupBox(reinterpret_cast<const char*>(u8"AI对话"), this);
    auto chat_layout = new QVBoxLayout(chat_group);

    chat_history_ = new QListWidget();
    chat_history_->setMaximumHeight(200);
    chat_layout->addWidget(chat_history_);

    response_display_ = new QTextEdit();
    response_display_->setReadOnly(true);
    response_display_->setPlaceholderText(reinterpret_cast<const char*>(u8"AI响应将显示在这里..."));
    response_display_->setMaximumHeight(150);
    chat_layout->addWidget(response_display_);

    main_layout->addWidget(chat_group);

    // 输入框
    input_box_ = new QTextEdit();
    input_box_->setPlaceholderText(reinterpret_cast<const char*>(u8"输入你的问题或指令..."));
    input_box_->setMaximumHeight(80);
    main_layout->addWidget(input_box_);

    // 发送按钮
    send_btn_ = new QPushButton(reinterpret_cast<const char*>(u8"发送"));
    connect(send_btn_, &QPushButton::clicked, this, &AISidebar::onSendMessage);
    main_layout->addWidget(send_btn_);

    // 加载动画占位
    loading_indicator_ = new QLabel(reinterpret_cast<const char*>(u8"加载中..."));
    loading_indicator_->setVisible(false);
    main_layout->addWidget(loading_indicator_);

    main_layout->addStretch();
    setLayout(main_layout);
}

void AISidebar::setDocumentContext(const QString& context) {
    current_context_ = context;
}

void AISidebar::setSelectedText(const QString& text) {
    selected_text_ = text;
}

void AISidebar::onContinueWriting() {
    if (current_context_.isEmpty()) {
        response_display_->setText(reinterpret_cast<const char*>(u8"请先输入一些文本"));
        return;
    }

    showLoading(true);
    stop_requested_ = false;
    
    // Request cursor position and text before cursor from editor
    emit requestCursorPositionAndText();
}

void AISidebar::onCursorPositionAndTextReceived(int position, const QString& text_before) {
    // This is called after requesting cursor position in onContinueWriting
    if (!isVisible()) {
        return;
    }
    
    continue_write_position_ = position;
    
    // Use text before cursor as prompt for continuation
    std::string prompt = ai_writer_->continueWritingPrompt(text_before.toUtf8().toStdString());
    
    // Start streaming without accumulating
    ai_client_->completeStream(prompt,
        [this](const ai::StreamChunk& chunk) {
            // Check if stop was requested
            if (stop_requested_) {
                return;
            }
            
            QString text = QString::fromUtf8(chunk.text.c_str());
            bool is_finish = chunk.is_finish;
            QMetaObject::invokeMethod(this, [this, text, is_finish]() {
                // Insert at the fixed position saved when continue writing started
                emit insertTextAtPosition(text, continue_write_position_);
                // Update position for next chunk
                continue_write_position_ += text.length();
                if (is_finish) {
                    showLoading(false);
                    continue_write_position_ = -1;
                }
            }, Qt::QueuedConnection);
        });
}

void AISidebar::stopGeneration() {
    stop_requested_ = true;
    if (ai_client_) {
        ai_client_->stop();
    }
    showLoading(false);
}

void AISidebar::onPolish() {
    if (selected_text_.isEmpty()) {
        response_display_->setText(reinterpret_cast<const char*>(u8"请先选中要润色的文本"));
        return;
    }

    showLoading(true);
    response_display_->clear();

    std::string prompt = ai_writer_->polishPrompt(selected_text_.toUtf8().toStdString());
    
    // Output polished text in AI response box (like summarize)
    ai_client_->completeStream(prompt,
        [this](const ai::StreamChunk& chunk) {
            QString text = QString::fromUtf8(chunk.text.c_str());
            bool is_finish = chunk.is_finish;
            QMetaObject::invokeMethod(this, [this, text, is_finish]() {
                response_display_->moveCursor(QTextCursor::End);
                response_display_->insertPlainText(text);
                if (is_finish) {
                    showLoading(false);
                }
            }, Qt::QueuedConnection);
        });
}

void AISidebar::onSummarize() {
    if (selected_text_.isEmpty()) {
        response_display_->setText(reinterpret_cast<const char*>(u8"请先选中要摘要的内容"));
        return;
    }

    showLoading(true);
    response_display_->clear();

    std::string prompt = ai_writer_->summarizePrompt(selected_text_.toUtf8().toStdString());
    
    ai_client_->completeStream(prompt,
        [this](const ai::StreamChunk& chunk) {
            QString text = QString::fromUtf8(chunk.text.c_str());
            bool is_finish = chunk.is_finish;
            QMetaObject::invokeMethod(this, [this, text, is_finish]() {
                response_display_->moveCursor(QTextCursor::End);
                response_display_->insertPlainText(text);
                if (is_finish) {
                    showLoading(false);
                }
            }, Qt::QueuedConnection);
        });
}

void AISidebar::onSendMessage() {
    QString message = input_box_->toPlainText();
    if (message.isEmpty()) return;

    addMessage(message, true);
    input_box_->clear();

    showLoading(true);
    response_display_->clear();

    ai_client_->completeStream(message.toUtf8().toStdString(),
        [this](const ai::StreamChunk& chunk) {
            QString text = QString::fromUtf8(chunk.text.c_str());
            bool is_finish = chunk.is_finish;
            QMetaObject::invokeMethod(this, [this, text, is_finish]() {
                onStreamChunk(text, is_finish);
            }, Qt::QueuedConnection);
        });
}

void AISidebar::onStreamChunk(const QString& chunk, bool is_finish) {
    response_display_->moveCursor(QTextCursor::End);
    response_display_->insertPlainText(chunk);
    if (is_finish) {
        showLoading(false);
    }
}

void AISidebar::addMessage(const QString& text, bool is_user) {
    auto item = new QListWidgetItem();
    item->setText((is_user ? "👤 " : "🤖 ") + text);
    chat_history_->addItem(item);
}

void AISidebar::showLoading(bool show) {
    loading_indicator_->setVisible(show);
}

void AISidebar::onTextChanged(const QString& text) {
    current_context_ = text;
}

void AISidebar::onTextSelected(const QString& selected) {
    selected_text_ = selected;
}

} // namespace gui
