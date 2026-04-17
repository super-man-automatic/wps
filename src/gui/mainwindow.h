#pragma once
#include <QMainWindow>
<<<<<<< HEAD
#include <QKeyEvent>
#include <QEvent>
#include <QFile>
#include <QString>
=======
>>>>>>> 5475e174cf498978befe4c9d3f475f4a1cc6c9bd
#include <memory>

namespace gui {

class EditorWidget;
class AISidebar;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

<<<<<<< HEAD
protected:
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

=======
>>>>>>> 5475e174cf498978befe4c9d3f475f4a1cc6c9bd
private slots:
    void onNewDocument();
    void onOpenDocument();
    void onSaveDocument();
<<<<<<< HEAD
    void onSaveAsDocument();
    void onClearDocument();
    void onAbout();
    void onFontSizeIncrease();
    void onFontSizeDecrease();
    void onToggleTheme();
    void updateStatusBar();
=======
    void onAbout();
>>>>>>> 5475e174cf498978befe4c9d3f475f4a1cc6c9bd

private:
    void setupUI();
    void setupMenuBar();
<<<<<<< HEAD
    void setupStatusBar();
    void setupConnections();
    void loadFile(const QString& path);
    void saveFile(const QString& path);

    std::unique_ptr<EditorWidget> editor_;
    std::unique_ptr<AISidebar> ai_sidebar_;
    
    QString current_file_path_;
    bool is_dark_theme_ = false;
    int current_font_size_ = 12;
=======
    void setupConnections();

    std::unique_ptr<EditorWidget> editor_;
    std::unique_ptr<AISidebar> ai_sidebar_;
>>>>>>> 5475e174cf498978befe4c9d3f475f4a1cc6c9bd
};

} // namespace gui
