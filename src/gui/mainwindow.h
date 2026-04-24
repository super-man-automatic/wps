#pragma once
#include <QMainWindow>
#include <QKeyEvent>
#include <QEvent>
#include <QFile>
#include <QString>
#ifdef ENABLE_IMAGE_FEATURES
#include <QImage>
#include <QMap>
#include <QList>
#endif
#include <memory>

namespace gui {

class EditorWidget;
class AISidebar;
#ifdef ENABLE_IMAGE_FEATURES
class ImageEditorDialog;
#endif

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onNewDocument();
    void onOpenDocument();
    void onSaveDocument();
    void onSaveAsDocument();
    void onClearDocument();
    void onAbout();
    void onFontSizeIncrease();
    void onFontSizeDecrease();
    void onToggleTheme();
    void updateStatusBar();
    
#ifdef ENABLE_IMAGE_FEATURES
    // Image editing slots
    void onInsertImage();
    void onInsertImageFromFile();
    void onEditImage();
    void onResizeImage();
    void onRotateImage();
    void onCropImage();
    void onApplyGrayscale();
    void onApplyBlur();
    void onApplySharpen();
#endif

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();
    void loadFile(const QString& path);
    void saveFile(const QString& path);

    std::unique_ptr<EditorWidget> editor_;
    std::unique_ptr<AISidebar> ai_sidebar_;
    
    QString current_file_path_;
    bool is_dark_theme_ = false;
    int current_font_size_ = 12;
    
#ifdef ENABLE_IMAGE_FEATURES
    // Image editing
    std::unique_ptr<ImageEditorDialog> image_editor_;
    QImage current_image_;
    bool has_image_ = false;
    
    // Image history for undo (image_name -> list of historical versions)
    QMap<QString, QList<QImage>> image_history_;
    void saveImageToHistory(const QString& img_name, const QImage& image);
    bool tryUndoImageEdit();
#endif
};

} // namespace gui
