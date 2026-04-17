#pragma once
#include <QMainWindow>
#include <memory>

namespace gui {

class EditorWidget;
class AISidebar;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onNewDocument();
    void onOpenDocument();
    void onSaveDocument();
    void onAbout();

private:
    void setupUI();
    void setupMenuBar();
    void setupConnections();

    std::unique_ptr<EditorWidget> editor_;
    std::unique_ptr<AISidebar> ai_sidebar_;
};

} // namespace gui
