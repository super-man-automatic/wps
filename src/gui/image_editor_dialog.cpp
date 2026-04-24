#include "image_editor_dialog.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QSplitter>

namespace gui {

ImageEditorDialog::ImageEditorDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Image Editor"));
    setMinimumSize(900, 700);
    resize(1000, 800);
    
    setupUI();
}

ImageEditorDialog::~ImageEditorDialog() = default;

void ImageEditorDialog::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // Splitter for image and controls
    auto* splitter = new QSplitter(Qt::Horizontal);
    
    // Image widget with scroll area
    auto* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    imageWidget_ = new ImageWidget(this);
    scrollArea->setWidget(imageWidget_);
    
    splitter->addWidget(scrollArea);
    
    // Control panel with tabs
    auto* controlPanel = new QWidget();
    auto* controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setContentsMargins(10, 10, 10, 10);
    
    auto* tabWidget = new QTabWidget();
    
    // Basic tab
    setupBasicTab();
    tabWidget->addTab(basicTab_, tr("Basic"));
    
    // Adjust tab
    setupAdjustTab();
    tabWidget->addTab(adjustTab_, tr("Adjust"));
    
    // Filter tab
    setupFilterTab();
    tabWidget->addTab(filterTab_, tr("Filters"));
    
    controlLayout->addWidget(tabWidget);
    
    // Undo/Redo buttons
    auto* historyLayout = new QHBoxLayout();
    btnUndo_ = new QPushButton(tr("Undo"), this);
    btnRedo_ = new QPushButton(tr("Redo"), this);
    btnReset_ = new QPushButton(tr("Reset"), this);
    btnResetOriginal_ = new QPushButton(tr("Reset to Original"), this);
    
    historyLayout->addWidget(btnUndo_);
    historyLayout->addWidget(btnRedo_);
    historyLayout->addStretch();
    historyLayout->addWidget(btnReset_);
    historyLayout->addWidget(btnResetOriginal_);
    
    controlLayout->addLayout(historyLayout);
    
    // Dialog buttons
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    btnAccept_ = buttonBox->button(QDialogButtonBox::Ok);
    btnReject_ = buttonBox->button(QDialogButtonBox::Cancel);
    btnAccept_->setText(tr("Apply"));
    btnReject_->setText(tr("Cancel"));
    
    controlLayout->addWidget(buttonBox);
    
    splitter->addWidget(controlPanel);
    splitter->setSizes({700, 300});
    
    mainLayout->addWidget(splitter);
    
    // Connect signals
    connect(btnZoomIn_, &QPushButton::clicked, this, &ImageEditorDialog::onZoomIn);
    connect(btnZoomOut_, &QPushButton::clicked, this, &ImageEditorDialog::onZoomOut);
    connect(btnZoomReset_, &QPushButton::clicked, this, &ImageEditorDialog::onZoomReset);
    connect(btnRotateLeft_, &QPushButton::clicked, this, &ImageEditorDialog::onRotateLeft);
    connect(btnRotateRight_, &QPushButton::clicked, this, &ImageEditorDialog::onRotateRight);
    connect(btnCropStart_, &QPushButton::clicked, this, &ImageEditorDialog::onCropStart);
    connect(btnCropApply_, &QPushButton::clicked, this, &ImageEditorDialog::onCropApply);
    connect(btnCropCancel_, &QPushButton::clicked, this, &ImageEditorDialog::onCropCancel);
    connect(btnResizeStart_, &QPushButton::clicked, this, &ImageEditorDialog::onResizeStart);
    connect(btnResizeApply_, &QPushButton::clicked, this, &ImageEditorDialog::onResizeApply);
    
    connect(spinWidth_, QOverload<int>::of(&QSpinBox::valueChanged), this, &ImageEditorDialog::onWidthChanged);
    connect(spinHeight_, QOverload<int>::of(&QSpinBox::valueChanged), this, &ImageEditorDialog::onHeightChanged);
    connect(chkMaintainAspect_, &QCheckBox::toggled, this, &ImageEditorDialog::onMaintainAspectRatioChanged);
    
    connect(sliderBrightness_, &QSlider::valueChanged, this, &ImageEditorDialog::onBrightnessChanged);
    connect(sliderContrast_, &QSlider::valueChanged, this, &ImageEditorDialog::onContrastChanged);
    connect(btnBrightnessReset_, &QPushButton::clicked, this, &ImageEditorDialog::onBrightnessContrastReset);
    
    connect(btnGrayscale_, &QPushButton::clicked, this, &ImageEditorDialog::onGrayscale);
    connect(btnBlur_, &QPushButton::clicked, this, &ImageEditorDialog::onBlur);
    connect(btnSharpen_, &QPushButton::clicked, this, &ImageEditorDialog::onSharpen);
    connect(btnEdgeDetection_, &QPushButton::clicked, this, &ImageEditorDialog::onEdgeDetection);
    connect(btnThreshold_, &QPushButton::clicked, this, &ImageEditorDialog::onThreshold);
    
    connect(btnUndo_, &QPushButton::clicked, this, &ImageEditorDialog::onUndo);
    connect(btnRedo_, &QPushButton::clicked, this, &ImageEditorDialog::onRedo);
    connect(btnReset_, &QPushButton::clicked, this, &ImageEditorDialog::onReset);
    connect(btnResetOriginal_, &QPushButton::clicked, this, &ImageEditorDialog::onResetToOriginal);
    
    connect(btnAccept_, &QPushButton::clicked, this, &ImageEditorDialog::onAccept);
    connect(btnReject_, &QPushButton::clicked, this, &ImageEditorDialog::onReject);
    
    connect(imageWidget_, &ImageWidget::imageChanged, this, &ImageEditorDialog::updateUndoRedoButtons);
    connect(imageWidget_, &ImageWidget::editModeChanged, this, &ImageEditorDialog::updateButtonStates);
}

void ImageEditorDialog::setupBasicTab() {
    basicTab_ = new QWidget();
    auto* layout = new QVBoxLayout(basicTab_);
    
    // Zoom group
    auto* zoomGroup = new QGroupBox(tr("Zoom"));
    auto* zoomLayout = new QHBoxLayout(zoomGroup);
    btnZoomIn_ = new QPushButton(tr("Zoom In (+)"), this);
    btnZoomOut_ = new QPushButton(tr("Zoom Out (-)"), this);
    btnZoomReset_ = new QPushButton(tr("Reset Zoom"), this);
    zoomLayout->addWidget(btnZoomIn_);
    zoomLayout->addWidget(btnZoomOut_);
    zoomLayout->addWidget(btnZoomReset_);
    layout->addWidget(zoomGroup);
    
    // Rotate group
    auto* rotateGroup = new QGroupBox(tr("Rotate"));
    auto* rotateLayout = new QHBoxLayout(rotateGroup);
    btnRotateLeft_ = new QPushButton(tr("Rotate Left (90°)"), this);
    btnRotateRight_ = new QPushButton(tr("Rotate Right (90°)"), this);
    rotateLayout->addWidget(btnRotateLeft_);
    rotateLayout->addWidget(btnRotateRight_);
    layout->addWidget(rotateGroup);
    
    // Crop group
    auto* cropGroup = new QGroupBox(tr("Crop"));
    auto* cropLayout = new QHBoxLayout(cropGroup);
    btnCropStart_ = new QPushButton(tr("Start Crop"), this);
    btnCropApply_ = new QPushButton(tr("Apply Crop"), this);
    btnCropCancel_ = new QPushButton(tr("Cancel"), this);
    cropLayout->addWidget(btnCropStart_);
    cropLayout->addWidget(btnCropApply_);
    cropLayout->addWidget(btnCropCancel_);
    layout->addWidget(cropGroup);
    
    // Resize group
    auto* resizeGroup = new QGroupBox(tr("Resize"));
    auto* resizeLayout = new QFormLayout(resizeGroup);
    
    auto* sizeInputLayout = new QHBoxLayout();
    spinWidth_ = new QSpinBox(this);
    spinHeight_ = new QSpinBox(this);
    spinWidth_->setRange(1, 10000);
    spinHeight_->setRange(1, 10000);
    sizeInputLayout->addWidget(new QLabel(tr("Width:")));
    sizeInputLayout->addWidget(spinWidth_);
    sizeInputLayout->addWidget(new QLabel(tr("Height:")));
    sizeInputLayout->addWidget(spinHeight_);
    
    chkMaintainAspect_ = new QCheckBox(tr("Maintain aspect ratio"));
    chkMaintainAspect_->setChecked(true);
    
    auto* resizeBtnLayout = new QHBoxLayout();
    btnResizeStart_ = new QPushButton(tr("Enable Resize"), this);
    btnResizeApply_ = new QPushButton(tr("Apply Resize"), this);
    resizeBtnLayout->addWidget(btnResizeStart_);
    resizeBtnLayout->addWidget(btnResizeApply_);
    
    resizeLayout->addRow(sizeInputLayout);
    resizeLayout->addRow(chkMaintainAspect_);
    resizeLayout->addRow(resizeBtnLayout);
    layout->addWidget(resizeGroup);
    
    layout->addStretch();
}

void ImageEditorDialog::setupAdjustTab() {
    adjustTab_ = new QWidget();
    auto* layout = new QVBoxLayout(adjustTab_);
    
    // Brightness group
    auto* brightnessGroup = new QGroupBox(tr("Brightness"));
    auto* brightnessLayout = new QVBoxLayout(brightnessGroup);
    
    auto* brightnessSliderLayout = new QHBoxLayout();
    sliderBrightness_ = new QSlider(Qt::Horizontal, this);
    sliderBrightness_->setRange(-100, 100);
    sliderBrightness_->setValue(0);
    lblBrightnessValue_ = new QLabel("0");
    brightnessSliderLayout->addWidget(new QLabel(tr("-100")));
    brightnessSliderLayout->addWidget(sliderBrightness_);
    brightnessSliderLayout->addWidget(new QLabel(tr("+100")));
    brightnessSliderLayout->addWidget(lblBrightnessValue_);
    
    brightnessLayout->addLayout(brightnessSliderLayout);
    layout->addWidget(brightnessGroup);
    
    // Contrast group
    auto* contrastGroup = new QGroupBox(tr("Contrast"));
    auto* contrastLayout = new QVBoxLayout(contrastGroup);
    
    auto* contrastSliderLayout = new QHBoxLayout();
    sliderContrast_ = new QSlider(Qt::Horizontal, this);
    sliderContrast_->setRange(0, 200);
    sliderContrast_->setValue(100);
    lblContrastValue_ = new QLabel("1.0");
    contrastSliderLayout->addWidget(new QLabel(tr("0.0")));
    contrastSliderLayout->addWidget(sliderContrast_);
    contrastSliderLayout->addWidget(new QLabel(tr("2.0")));
    contrastSliderLayout->addWidget(lblContrastValue_);
    
    contrastLayout->addLayout(contrastSliderLayout);
    layout->addWidget(contrastGroup);
    
    // Reset button
    btnBrightnessReset_ = new QPushButton(tr("Reset Brightness & Contrast"), this);
    layout->addWidget(btnBrightnessReset_);
    
    layout->addStretch();
}

void ImageEditorDialog::setupFilterTab() {
    filterTab_ = new QWidget();
    auto* layout = new QVBoxLayout(filterTab_);
    
    // Quick filters
    auto* quickFilterGroup = new QGroupBox(tr("Quick Filters"));
    auto* quickFilterLayout = new QVBoxLayout(quickFilterGroup);
    
    btnGrayscale_ = new QPushButton(tr("Grayscale"), this);
    btnSharpen_ = new QPushButton(tr("Sharpen"), this);
    btnEdgeDetection_ = new QPushButton(tr("Edge Detection"), this);
    
    quickFilterLayout->addWidget(btnGrayscale_);
    quickFilterLayout->addWidget(btnSharpen_);
    quickFilterLayout->addWidget(btnEdgeDetection_);
    layout->addWidget(quickFilterGroup);
    
    // Blur filter
    auto* blurGroup = new QGroupBox(tr("Blur"));
    auto* blurLayout = new QVBoxLayout(blurGroup);
    
    auto* blurSliderLayout = new QHBoxLayout();
    sliderBlur_ = new QSlider(Qt::Horizontal, this);
    sliderBlur_->setRange(1, 21);
    sliderBlur_->setValue(1);
    sliderBlur_->setTickInterval(2);
    sliderBlur_->setTickPosition(QSlider::TicksBelow);
    lblBlurValue_ = new QLabel("1");
    blurSliderLayout->addWidget(sliderBlur_);
    blurSliderLayout->addWidget(lblBlurValue_);
    
    btnBlur_ = new QPushButton(tr("Apply Blur"), this);
    
    blurLayout->addLayout(blurSliderLayout);
    blurLayout->addWidget(btnBlur_);
    layout->addWidget(blurGroup);
    
    // Threshold filter
    auto* thresholdGroup = new QGroupBox(tr("Threshold"));
    auto* thresholdLayout = new QVBoxLayout(thresholdGroup);
    
    auto* thresholdSliderLayout = new QHBoxLayout();
    sliderThreshold_ = new QSlider(Qt::Horizontal, this);
    sliderThreshold_->setRange(0, 255);
    sliderThreshold_->setValue(128);
    lblThresholdValue_ = new QLabel("128");
    thresholdSliderLayout->addWidget(sliderThreshold_);
    thresholdSliderLayout->addWidget(lblThresholdValue_);
    
    btnThreshold_ = new QPushButton(tr("Apply Threshold"), this);
    
    thresholdLayout->addLayout(thresholdSliderLayout);
    thresholdLayout->addWidget(btnThreshold_);
    layout->addWidget(thresholdGroup);
    
    layout->addStretch();
}

bool ImageEditorDialog::loadImage(const QString& filePath) {
    if (!imageWidget_->loadImage(filePath)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to load image: %1").arg(filePath));
        return false;
    }
    
    updateSizeControls();
    updateUndoRedoButtons();
    return true;
}

bool ImageEditorDialog::loadImage(const QImage& image) {
    if (!imageWidget_->insertImage(image)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to load image"));
        return false;
    }
    
    updateSizeControls();
    updateUndoRedoButtons();
    return true;
}

QImage ImageEditorDialog::getResultImage() const {
    // Return the actual processed image, not a widget grab
    return imageWidget_->getCurrentImage();
}

bool ImageEditorDialog::saveResult(const QString& filePath) {
    return imageWidget_->saveImage(filePath);
}

void ImageEditorDialog::onZoomIn() {
    imageWidget_->scaleImage(1.25);
}

void ImageEditorDialog::onZoomOut() {
    imageWidget_->scaleImage(0.8);
}

void ImageEditorDialog::onZoomReset() {
    while (imageWidget_->canUndo()) {
        imageWidget_->undo();
    }
}

void ImageEditorDialog::onRotateLeft() {
    imageWidget_->rotateImage(-90);
}

void ImageEditorDialog::onRotateRight() {
    imageWidget_->rotateImage(90);
}

void ImageEditorDialog::onRotateCustom() {
    // TODO: Implement custom rotation dialog
    // For now, just rotate by 45 degrees as a placeholder
    imageWidget_->rotateImage(45);
}

void ImageEditorDialog::onResizeStart() {
    imageWidget_->setEditMode(ImageEditMode::Resize);
}

void ImageEditorDialog::onResizeApply() {
    int width = spinWidth_->value();
    int height = spinHeight_->value();
    imageWidget_->resizeImage(width, height);
    imageWidget_->setEditMode(ImageEditMode::None);
}

void ImageEditorDialog::onWidthChanged(int value) {
    if (updatingSizeControls_) return;
    if (chkMaintainAspect_->isChecked() && aspectRatio_ > 0) {
        updatingSizeControls_ = true;
        spinHeight_->setValue(static_cast<int>(value / aspectRatio_));
        updatingSizeControls_ = false;
    }
}

void ImageEditorDialog::onHeightChanged(int value) {
    if (updatingSizeControls_) return;
    if (chkMaintainAspect_->isChecked() && aspectRatio_ > 0) {
        updatingSizeControls_ = true;
        spinWidth_->setValue(static_cast<int>(value * aspectRatio_));
        updatingSizeControls_ = false;
    }
}

void ImageEditorDialog::onMaintainAspectRatioChanged(bool checked) {
    if (checked && spinHeight_->value() > 0) {
        aspectRatio_ = static_cast<double>(spinWidth_->value()) / spinHeight_->value();
    }
}

void ImageEditorDialog::onCropStart() {
    imageWidget_->startCrop();
}

void ImageEditorDialog::onCropApply() {
    imageWidget_->applyCrop();
}

void ImageEditorDialog::onCropCancel() {
    imageWidget_->cancelCrop();
}

void ImageEditorDialog::onGrayscale() {
    imageWidget_->applyGrayscale();
}

void ImageEditorDialog::onBlur() {
    int kernelSize = sliderBlur_->value();
    imageWidget_->applyBlur(kernelSize);
}

void ImageEditorDialog::onSharpen() {
    imageWidget_->applySharpen();
}

void ImageEditorDialog::onEdgeDetection() {
    imageWidget_->applyEdgeDetection();
}

void ImageEditorDialog::onThreshold() {
    int threshold = sliderThreshold_->value();
    imageWidget_->applyThreshold(threshold);
}

void ImageEditorDialog::onBrightnessChanged(int value) {
    lblBrightnessValue_->setText(QString::number(value));
    // Apply brightness in real-time or wait for release
    // For now, we'll apply on slider release or use a timer
}

void ImageEditorDialog::onContrastChanged(int value) {
    double contrast = value / 100.0;
    lblContrastValue_->setText(QString::number(contrast, 'f', 1));
    // Apply contrast
    if (imageWidget_->hasImage()) {
        imageWidget_->adjustContrast(contrast);
    }
}

void ImageEditorDialog::onBrightnessContrastReset() {
    sliderBrightness_->setValue(0);
    sliderContrast_->setValue(100);
    imageWidget_->resetImage();
}

void ImageEditorDialog::onUndo() {
    imageWidget_->undo();
    updateUndoRedoButtons();
}

void ImageEditorDialog::onRedo() {
    imageWidget_->redo();
    updateUndoRedoButtons();
}

void ImageEditorDialog::updateUndoRedoButtons() {
    btnUndo_->setEnabled(imageWidget_->canUndo());
    btnRedo_->setEnabled(imageWidget_->canRedo());
}

void ImageEditorDialog::onReset() {
    imageWidget_->resetImage();
}

void ImageEditorDialog::onResetToOriginal() {
    imageWidget_->resetToOriginal();
    updateSizeControls();
    updateUndoRedoButtons();
}

void ImageEditorDialog::onAccept() {
    accept();
}

void ImageEditorDialog::onReject() {
    reject();
}

// Direct filter application methods for MainWindow
void ImageEditorDialog::applyGrayscale() {
    imageWidget_->applyGrayscale();
}

void ImageEditorDialog::applyBlur(int kernelSize) {
    imageWidget_->applyBlur(kernelSize);
}

void ImageEditorDialog::applySharpen() {
    imageWidget_->applySharpen();
}

void ImageEditorDialog::updateSizeControls() {
    if (!imageWidget_->hasImage()) return;
    
    updatingSizeControls_ = true;
    
    QSize size = imageWidget_->imageSize();
    originalWidth_ = size.width();
    originalHeight_ = size.height();
    
    spinWidth_->setValue(originalWidth_);
    spinHeight_->setValue(originalHeight_);
    
    if (originalHeight_ > 0) {
        aspectRatio_ = static_cast<double>(originalWidth_) / originalHeight_;
    }
    
    updatingSizeControls_ = false;
}

void ImageEditorDialog::updateButtonStates() {
    ImageEditMode mode = imageWidget_->editMode();
    
    // Update crop buttons
    bool isCropping = (mode == ImageEditMode::Crop);
    btnCropApply_->setEnabled(isCropping);
    btnCropCancel_->setEnabled(isCropping);
    
    // Update resize buttons
    bool isResizing = (mode == ImageEditMode::Resize);
    btnResizeApply_->setEnabled(isResizing);
}

} // namespace gui
