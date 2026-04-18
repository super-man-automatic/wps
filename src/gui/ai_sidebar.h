#pragma once
#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QListWidget>
#include <memory>
#include "../ai/llm_client.h"
#include "../ai/ai_writer.h"

namespace gui {

class AISidebar : public QWidget {
    Q_OBJECT
public:
    explicit AISidebar(QWidget* parent = nullptr);
    ~AISidebar();

    void setDocumentContext(const QString& context);
    void setSelectedText(const QString& text);

signals:
    void insertTextRequested(const QString& text);
    void insertTextAtPosition(const QString& text, int position);
    void requestCursorPositionAndText();

public slots:
    void onTextChanged(const QString& text);
    void onTextSelected(const QString& selected);
    void onCursorPositionAndTextReceived(int position, const QString& text_before);
    void stopGeneration();

private slots:
    void onContinueWriting();
    void onPolish();
    void onSummarize();
    void onSendMessage();
    void onStreamChunk(const QString& chunk, bool is_finish);

private:
    void setupUI();
    void addMessage(const QString& text, bool is_user);
    void showLoading(bool show);

    QListWidget* chat_history_;
    QTextEdit* input_box_;
    QTextEdit* response_display_;
    QPushButton* continue_btn_;
    QPushButton* polish_btn_;
    QPushButton* summarize_btn_;
    QPushButton* send_btn_;
    QWidget* loading_indicator_;

    std::shared_ptr<ai::LLMClient> ai_client_;
    std::shared_ptr<ai::AIWriter> ai_writer_;

    QString current_context_;
    QString selected_text_;
    QString accumulated_text_;
    int continue_write_position_ = -1;
    bool stop_requested_ = false;
};

} // namespace gui
