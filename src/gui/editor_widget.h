#pragma once
#include <QPlainTextEdit>
#include <memory>

namespace smartdoc {
class Document;
}

namespace gui {

class EditorWidget : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit EditorWidget(QWidget* parent = nullptr);
    ~EditorWidget();

    void openFile(const std::string& path);
    void saveFile();
    std::string getText() const;
    void setText(const std::string& text);

signals:
    void contentChanged(const QString& text);
    void selectionChanged(const QString& selected_text);

private slots:
    void onTextChanged();
    void onSelectionChanged();

private:
    std::shared_ptr<smartdoc::Document> document_;
    std::string current_file_path_;
};

} // namespace gui
