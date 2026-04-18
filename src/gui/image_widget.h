#pragma once

#include <QWidget>
#include <QPixmap>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <memory>

namespace gui {

// 图片操作模式
enum class ImageEditMode {
    None,       // 无操作
    Resize,     // 调整大小
    Rotate,     // 旋转
    Crop        // 裁剪
};

// 图片数据类，包含原始 OpenCV 图片和显示相关属性
struct ImageData {
    cv::Mat originalImage;      // 原始 OpenCV 图片
    cv::Mat currentImage;       // 当前处理的图片
    QPixmap displayPixmap;      // 用于显示的 QPixmap
    double scaleFactor = 1.0;   // 缩放比例
    int rotationAngle = 0;      // 旋转角度
    QRect cropRect;             // 裁剪区域
    QString filePath;           // 文件路径
    
    ImageData() = default;
    explicit ImageData(const cv::Mat& img) : originalImage(img.clone()), currentImage(img.clone()) {}
    
    void reset() {
        currentImage = originalImage.clone();
        scaleFactor = 1.0;
        rotationAngle = 0;
        cropRect = QRect();
    }
    
    void applyChanges() {
        originalImage = currentImage.clone();
    }
};

class ImageWidget : public QWidget {
    Q_OBJECT

public:
    explicit ImageWidget(QWidget* parent = nullptr);
    ~ImageWidget() override;

    // 加载和保存
    bool loadImage(const QString& filePath);
    bool saveImage(const QString& filePath);
    bool insertImage(const QImage& image);
    
    // 图片属性
    bool hasImage() const { return imageData_ && !imageData_->currentImage.empty(); }
    QSize imageSize() const;
    QString filePath() const { return imageData_ ? imageData_->filePath : QString(); }
    
    // 编辑操作
    void setEditMode(ImageEditMode mode);
    ImageEditMode editMode() const { return editMode_; }
    
    // 大小调整
    void resizeImage(int width, int height);
    void scaleImage(double factor);
    double scaleFactor() const { return imageData_ ? imageData_->scaleFactor : 1.0; }
    
    // 旋转
    void rotateImage(int degrees);
    int rotationAngle() const { return imageData_ ? imageData_->rotationAngle : 0; }
    
    // 裁剪
    void startCrop();
    void setCropRect(const QRect& rect);
    void applyCrop();
    void cancelCrop();
    
    // OpenCV 高级功能
    void applyGrayscale();
    void applyBlur(int kernelSize);
    void applySharpen();
    void adjustBrightness(int value);
    void adjustContrast(double value);
    void applyEdgeDetection();
    void applyThreshold(int thresholdValue);
    
    // 撤销/重做
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
    
    // 重置
    void resetImage();
    void resetToOriginal();

signals:
    void imageChanged();
    void scaleChanged(double factor);
    void rotationChanged(int angle);
    void editModeChanged(ImageEditMode mode);
    void cropStarted();
    void cropFinished(bool applied);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    // 图片处理
    void updateDisplay();
    QPixmap cvMatToQPixmap(const cv::Mat& mat);
    cv::Mat qImageToCvMat(const QImage& image);
    cv::Mat rotateMat(const cv::Mat& src, int degrees);
    cv::Mat cropMat(const cv::Mat& src, const QRect& rect);
    
    // 绘制
    void drawResizeHandles(QPainter& painter);
    void drawCropOverlay(QPainter& painter);
    void drawRotationIndicator(QPainter& painter);
    
    // 坐标转换
    QPoint widgetToImage(const QPoint& widgetPos) const;
    QRect widgetToImage(const QRect& widgetRect) const;
    QPoint imageToWidget(const QPoint& imagePos) const;
    QRect imageToWidget(const QRect& imageRect) const;
    
    // 历史记录
    void pushHistory();
    void popHistory();
    
    // 成员变量
    std::unique_ptr<ImageData> imageData_;
    ImageEditMode editMode_ = ImageEditMode::None;
    
    // 历史记录用于撤销/重做
    std::vector<std::unique_ptr<ImageData>> history_;
    size_t historyIndex_ = 0;
    static constexpr size_t MAX_HISTORY = 20;
    
    // 交互状态
    bool isDragging_ = false;
    bool isResizing_ = false;
    bool isCropping_ = false;
    QPoint dragStartPos_;
    QPoint resizeStartPos_;
    QRect resizeStartRect_;
    int resizeHandle_ = -1;  // 0-7 表示8个方向的手柄
    
    // 显示设置
    Qt::AspectRatioMode aspectRatioMode_ = Qt::KeepAspectRatio;
    bool smoothTransformation_ = true;
};

} // namespace gui
