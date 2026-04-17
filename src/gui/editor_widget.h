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
<<<<<<< HEAD
    void contentChanged(const QString& text);
=======
    void textChanged(const QString& text);
>>>>>>> 5475e174cf498978befe4c9d3f475f4a1cc6c9bd
    void selectionChanged(const QString& selected_text);

private slots:
    void onTextChanged();
<<<<<<< HEAD
    void onSelectionChanged();
=======
>>>>>>> 5475e174cf498978befe4c9d3f475f4a1cc6c9bd

private:
    std::shared_ptr<smartdoc::Document> document_;
    std::string current_file_path_;
};

} // namespace gui
