#include "ai_sidebar.h"
#include <QLabel>
#include <QGroupBox>
#include <QtConcurrent>
#include <QThread>

namespace gui {

AISidebar::AISidebar(QWidget* parent)
    : QWidget(parent) {
    // 初始化AI客户端（使用Mock后端）
    ai_client_ = std::make_shared<ai::LLMClient>(ai::Backend::Mock);
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
    auto title = new QLabel("🤖 AI助手");
    title->setStyleSheet("font-size: 14px; font-weight: bold;");
    main_layout->addWidget(title);

    // AI功能按钮组
    auto func_group = new QGroupBox("快速功能", this);
    auto func_layout = new QVBoxLayout(func_group);

    continue_btn_ = new QPushButton("📝 续写");
    connect(continue_btn_, &QPushButton::clicked, this, &AISidebar::onContinueWriting);
    func_layout->addWidget(continue_btn_);

    polish_btn_ = new QPushButton("✏️ 润色");
    connect(polish_btn_, &QPushButton::clicked, this, &AISidebar::onPolish);
    func_layout->addWidget(polish_btn_);

    summarize_btn_ = new QPushButton("📋 摘要");
    connect(summarize_btn_, &QPushButton::clicked, this, &AISidebar::onSummarize);
    func_layout->addWidget(summarize_btn_);

    main_layout->addWidget(func_group);

    // 聊天区域
    auto chat_group = new QGroupBox("AI对话", this);
    auto chat_layout = new QVBoxLayout(chat_group);

    chat_history_ = new QListWidget();
    chat_history_->setMaximumHeight(200);
    chat_layout->addWidget(chat_history_);

    response_display_ = new QTextEdit();
    response_display_->setReadOnly(true);
    response_display_->setPlaceholderText("AI响应将显示在这里...");
    response_display_->setMaximumHeight(150);
    chat_layout->addWidget(response_display_);

    main_layout->addWidget(chat_group);

    // 输入框
    input_box_ = new QTextEdit();
    input_box_->setPlaceholderText("输入你的问题或指令...");
    input_box_->setMaximumHeight(80);
    main_layout->addWidget(input_box_);

    // 发送按钮
    send_btn_ = new QPushButton("发送");
    connect(send_btn_, &QPushButton::clicked, this, &AISidebar::onSendMessage);
    main_layout->addWidget(send_btn_);

    // 加载动画占位
    loading_indicator_ = new QLabel("加载中...");
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
        response_display_->setText("请先输入一些文本");
        return;
    }

    showLoading(true);
    response_display_->clear();

    auto prompt = current_context_;
    ai_client_->completeStream(prompt.toStdString(),
        [this](const ai::StreamChunk& chunk) {
            QString text = QString::fromStdString(chunk.text);
            QMetaObject::invokeMethod(this, [this, text]() {
                onStreamChunk(text);
            }, Qt::QueuedConnection);
        });
}

void AISidebar::onPolish() {
    if (selected_text_.isEmpty()) {
        response_display_->setText("请先选中要润色的文本");
        return;
    }

    showLoading(true);
    response_display_->clear();

    // 在后台线程执行
    QtConcurrent::run([this]() {
        auto result = ai_writer_->polish(selected_text_.toStdString());
        QMetaObject::invokeMethod(this, [this, result]() {
            response_display_->setText(QString::fromStdString(result));
            showLoading(false);
        }, Qt::QueuedConnection);
    });
}

void AISidebar::onSummarize() {
    if (current_context_.isEmpty()) {
        response_display_->setText("请先输入要摘要的内容");
        return;
    }

    showLoading(true);
    response_display_->clear();

    QtConcurrent::run([this]() {
        auto result = ai_writer_->summarize(current_context_.toStdString());
        QMetaObject::invokeMethod(this, [this, result]() {
            response_display_->setText(QString::fromStdString(result));
            showLoading(false);
        }, Qt::QueuedConnection);
    });
}

void AISidebar::onSendMessage() {
    QString message = input_box_->toPlainText();
    if (message.isEmpty()) return;

    addMessage(message, true);
    input_box_->clear();

    showLoading(true);

    ai_client_->completeStream(message.toStdString(),
        [this](const ai::StreamChunk& chunk) {
            QString text = QString::fromStdString(chunk.text);
            QMetaObject::invokeMethod(this, [this, text]() {
                onStreamChunk(text);
            }, Qt::QueuedConnection);
        });
}

void AISidebar::onStreamChunk(const QString& chunk) {
    response_display_->moveCursor(QTextCursor::End);
    response_display_->insertPlainText(chunk);
    showLoading(false);
}

void AISidebar::addMessage(const QString& text, bool is_user) {
    auto item = new QListWidgetItem();
    item->setText((is_user ? "👤 " : "🤖 ") + text);
    chat_history_->addItem(item);
}

void AISidebar::showLoading(bool show) {
    loading_indicator_->setVisible(show);
}

void AISidebar::onTextSelected(const QString& selected) {
    selected_text_ = selected;
}

} // namespace gui
