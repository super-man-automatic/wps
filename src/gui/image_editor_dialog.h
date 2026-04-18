#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QTabWidget>
#include <QScrollArea>
#include <QCheckBox>
#include "image_widget.h"

namespace gui {

class ImageEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImageEditorDialog(QWidget* parent = nullptr);
    ~ImageEditorDialog() override;

    // 加载图片
    bool loadImage(const QString& filePath);
    bool loadImage(const QImage& image);
    
    // 获取结果
    QImage getResultImage() const;
    bool saveResult(const QString& filePath);
    
    // 直接应用滤镜（供 MainWindow 调用）
    void applyGrayscale();
    void applyBlur(int kernelSize);
    void applySharpen();

private slots:
    // 基本操作
    void onZoomIn();
    void onZoomOut();
    void onZoomReset();
    void onRotateLeft();
    void onRotateRight();
    void onRotateCustom();
    
    // 调整大小
    void onResizeStart();
    void onResizeApply();
    void onWidthChanged(int value);
    void onHeightChanged(int value);
    void onMaintainAspectRatioChanged(bool checked);
    
    // 裁剪
    void onCropStart();
    void onCropApply();
    void onCropCancel();
    
    // OpenCV 滤镜
    void onGrayscale();
    void onBlur();
    void onSharpen();
    void onEdgeDetection();
    void onThreshold();
    
    // 亮度和对比度
    void onBrightnessChanged(int value);
    void onContrastChanged(int value);
    void onBrightnessContrastReset();
    
    // 撤销重做
    void onUndo();
    void onRedo();
    void updateUndoRedoButtons();
    
    // 重置
    void onReset();
    void onResetToOriginal();
    
    // 接受/拒绝
    void onAccept();
    void onReject();

private:
    void setupUI();
    void setupBasicTab();
    void setupAdjustTab();
    void setupFilterTab();
    void updateSizeControls();
    void updateButtonStates();

    ImageWidget* imageWidget_;
    
    // 控制按钮
    QPushButton* btnZoomIn_;
    QPushButton* btnZoomOut_;
    QPushButton* btnZoomReset_;
    QPushButton* btnRotateLeft_;
    QPushButton* btnRotateRight_;
    QPushButton* btnCropStart_;
    QPushButton* btnCropApply_;
    QPushButton* btnCropCancel_;
    QPushButton* btnResizeStart_;
    QPushButton* btnResizeApply_;
    QPushButton* btnUndo_;
    QPushButton* btnRedo_;
    QPushButton* btnReset_;
    QPushButton* btnResetOriginal_;
    
    // 大小调整控件
    QSpinBox* spinWidth_;
    QSpinBox* spinHeight_;
    QCheckBox* chkMaintainAspect_;
    bool updatingSizeControls_ = false;
    double aspectRatio_ = 1.0;
    
    // 亮度对比度控件
    QSlider* sliderBrightness_;
    QSlider* sliderContrast_;
    QLabel* lblBrightnessValue_;
    QLabel* lblContrastValue_;
    QPushButton* btnBrightnessReset_;
    
    // 滤镜控件
    QSlider* sliderBlur_;
    QSlider* sliderThreshold_;
    QLabel* lblBlurValue_;
    QLabel* lblThresholdValue_;
    
    // Tab widgets
    QWidget* basicTab_ = nullptr;
    QWidget* adjustTab_ = nullptr;
    QWidget* filterTab_ = nullptr;
    
    // Filter buttons
    QPushButton* btnGrayscale_ = nullptr;
    QPushButton* btnBlur_ = nullptr;
    QPushButton* btnSharpen_ = nullptr;
    QPushButton* btnEdgeDetection_ = nullptr;
    QPushButton* btnThreshold_ = nullptr;
    
    // Accept/Reject buttons
    QPushButton* btnAccept_ = nullptr;
    QPushButton* btnReject_ = nullptr;
    
    // 原始尺寸
    int originalWidth_ = 0;
    int originalHeight_ = 0;
};

} // namespace gui
