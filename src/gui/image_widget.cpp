#include "image_widget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

namespace gui {

ImageWidget::ImageWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(100, 100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

ImageWidget::~ImageWidget() = default;

bool ImageWidget::loadImage(const QString& filePath) {
    cv::Mat image = cv::imread(filePath.toStdString(), cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        return false;
    }
    
    // BGR to RGB/RGBA if needed
    if (image.channels() == 3) {
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    } else if (image.channels() == 4) {
        cv::cvtColor(image, image, cv::COLOR_BGRA2RGBA);
    }
    
    imageData_ = std::make_unique<ImageData>(image);
    imageData_->filePath = filePath;
    
    history_.clear();
    historyIndex_ = 0;
    pushHistory();
    
    updateDisplay();
    emit imageChanged();
    return true;
}

bool ImageWidget::saveImage(const QString& filePath) {
    if (!hasImage()) return false;
    
    cv::Mat imageToSave = imageData_->currentImage.clone();
    
    // Convert back to BGR for saving
    if (imageToSave.channels() == 3) {
        cv::cvtColor(imageToSave, imageToSave, cv::COLOR_RGB2BGR);
    } else if (imageToSave.channels() == 4) {
        cv::cvtColor(imageToSave, imageToSave, cv::COLOR_RGBA2BGRA);
    }
    
    bool success = cv::imwrite(filePath.toStdString(), imageToSave);
    if (success) {
        imageData_->filePath = filePath;
    }
    return success;
}

bool ImageWidget::insertImage(const QImage& image) {
    if (image.isNull()) return false;
    
    cv::Mat mat = qImageToCvMat(image);
    if (mat.empty()) return false;
    
    imageData_ = std::make_unique<ImageData>(mat);
    
    history_.clear();
    historyIndex_ = 0;
    pushHistory();
    
    updateDisplay();
    emit imageChanged();
    return true;
}

QSize ImageWidget::imageSize() const {
    if (!hasImage()) return QSize();
    return QSize(imageData_->currentImage.cols, imageData_->currentImage.rows);
}

void ImageWidget::setEditMode(ImageEditMode mode) {
    if (editMode_ == mode) return;
    
    // Cancel current operation
    if (editMode_ == ImageEditMode::Crop) {
        cancelCrop();
    }
    
    editMode_ = mode;
    update();
    emit editModeChanged(mode);
}

void ImageWidget::resizeImage(int width, int height) {
    if (!hasImage()) return;
    
    pushHistory();
    
    cv::Mat resized;
    cv::resize(imageData_->currentImage, resized, cv::Size(width, height), 
               0, 0, smoothTransformation_ ? cv::INTER_LANCZOS4 : cv::INTER_NEAREST);
    imageData_->currentImage = resized;
    imageData_->scaleFactor = 1.0;
    
    updateDisplay();
    emit imageChanged();
}

void ImageWidget::scaleImage(double factor) {
    if (!hasImage() || factor <= 0) return;
    
    pushHistory();
    
    int newWidth = static_cast<int>(imageData_->currentImage.cols * factor);
    int newHeight = static_cast<int>(imageData_->currentImage.rows * factor);
    
    cv::Mat resized;
    cv::resize(imageData_->currentImage, resized, cv::Size(newWidth, newHeight),
               0, 0, smoothTransformation_ ? cv::INTER_LANCZOS4 : cv::INTER_NEAREST);
    imageData_->currentImage = resized;
    imageData_->scaleFactor *= factor;
    
    updateDisplay();
    emit imageChanged();
    emit scaleChanged(imageData_->scaleFactor);
}

void ImageWidget::rotateImage(int degrees) {
    if (!hasImage()) return;
    
    pushHistory();
    
    imageData_->currentImage = rotateMat(imageData_->currentImage, degrees);
    imageData_->rotationAngle = (imageData_->rotationAngle + degrees) % 360;
    
    updateDisplay();
    emit imageChanged();
    emit rotationChanged(imageData_->rotationAngle);
}

void ImageWidget::startCrop() {
    if (!hasImage()) return;
    setEditMode(ImageEditMode::Crop);
    isCropping_ = true;
    imageData_->cropRect = QRect(0, 0, 
                                  imageData_->currentImage.cols / 2,
                                  imageData_->currentImage.rows / 2);
    update();
    emit cropStarted();
}

void ImageWidget::setCropRect(const QRect& rect) {
    if (!hasImage()) return;
    imageData_->cropRect = rect;
    update();
}

void ImageWidget::applyCrop() {
    if (!hasImage() || imageData_->cropRect.isEmpty()) return;
    
    pushHistory();
    
    cv::Mat cropped = cropMat(imageData_->currentImage, imageData_->cropRect);
    if (!cropped.empty()) {
        imageData_->currentImage = cropped;
        imageData_->cropRect = QRect();
        isCropping_ = false;
        setEditMode(ImageEditMode::None);
        
        updateDisplay();
        emit imageChanged();
        emit cropFinished(true);
    }
}

void ImageWidget::cancelCrop() {
    if (!hasImage()) return;
    
    imageData_->cropRect = QRect();
    isCropping_ = false;
    setEditMode(ImageEditMode::None);
    update();
    emit cropFinished(false);
}

void ImageWidget::applyGrayscale() {
    if (!hasImage()) return;
    
    pushHistory();
    
    cv::Mat gray;
    if (imageData_->currentImage.channels() == 3) {
        cv::cvtColor(imageData_->currentImage, gray, cv::COLOR_RGB2GRAY);
    } else if (imageData_->currentImage.channels() == 4) {
        cv::cvtColor(imageData_->currentImage, gray, cv::COLOR_RGBA2GRAY);
    } else {
        gray = imageData_->currentImage.clone();
    }
    
    // Convert back to 3 channel for display consistency
    cv::cvtColor(gray, imageData_->currentImage, cv::COLOR_GRAY2RGB);
    
    updateDisplay();
    emit imageChanged();
}

void ImageWidget::applyBlur(int kernelSize) {
    if (!hasImage() || kernelSize < 1) return;
    
    pushHistory();
    
    if (kernelSize % 2 == 0) kernelSize++; // Must be odd
    
    cv::GaussianBlur(imageData_->currentImage, imageData_->currentImage,
                    cv::Size(kernelSize, kernelSize), 0);
    
    updateDisplay();
    emit imageChanged();
}

void ImageWidget::applySharpen() {
    if (!hasImage()) return;
    
    pushHistory();
    
    cv::Mat kernel = (cv::Mat_<float>(3,3) <<
        0, -1, 0,
        -1, 5, -1,
        0, -1, 0);
    
    cv::filter2D(imageData_->currentImage, imageData_->currentImage, -1, kernel);
    
    updateDisplay();
    emit imageChanged();
}

void ImageWidget::adjustBrightness(int value) {
    if (!hasImage()) return;
    
    pushHistory();
    
    imageData_->currentImage.convertTo(imageData_->currentImage, -1, 1.0, value);
    
    updateDisplay();
    emit imageChanged();
}

void ImageWidget::adjustContrast(double value) {
    if (!hasImage()) return;
    
    pushHistory();
    
    imageData_->currentImage.convertTo(imageData_->currentImage, -1, value, 0);
    
    updateDisplay();
    emit imageChanged();
}

void ImageWidget::applyEdgeDetection() {
    if (!hasImage()) return;
    
    pushHistory();
    
    cv::Mat gray, edges;
    if (imageData_->currentImage.channels() == 3) {
        cv::cvtColor(imageData_->currentImage, gray, cv::COLOR_RGB2GRAY);
    } else if (imageData_->currentImage.channels() == 4) {
        cv::cvtColor(imageData_->currentImage, gray, cv::COLOR_RGBA2GRAY);
    } else {
        gray = imageData_->currentImage;
    }
    
    cv::Canny(gray, edges, 100, 200);
    cv::cvtColor(edges, imageData_->currentImage, cv::COLOR_GRAY2RGB);
    
    updateDisplay();
    emit imageChanged();
}

void ImageWidget::applyThreshold(int thresholdValue) {
    if (!hasImage()) return;
    
    pushHistory();
    
    cv::Mat gray, thresholded;
    if (imageData_->currentImage.channels() == 3) {
        cv::cvtColor(imageData_->currentImage, gray, cv::COLOR_RGB2GRAY);
    } else if (imageData_->currentImage.channels() == 4) {
        cv::cvtColor(imageData_->currentImage, gray, cv::COLOR_RGBA2GRAY);
    } else {
        gray = imageData_->currentImage;
    }
    
    cv::threshold(gray, thresholded, thresholdValue, 255, cv::THRESH_BINARY);
    cv::cvtColor(thresholded, imageData_->currentImage, cv::COLOR_GRAY2RGB);
    
    updateDisplay();
    emit imageChanged();
}

void ImageWidget::undo() {
    if (!canUndo()) return;
    
    if (historyIndex_ > 0) {
        historyIndex_--;
        imageData_ = std::make_unique<ImageData>(*history_[historyIndex_]);
        updateDisplay();
        emit imageChanged();
    }
}

void ImageWidget::redo() {
    if (!canRedo()) return;
    
    if (historyIndex_ < history_.size() - 1) {
        historyIndex_++;
        imageData_ = std::make_unique<ImageData>(*history_[historyIndex_]);
        updateDisplay();
        emit imageChanged();
    }
}

bool ImageWidget::canUndo() const {
    return historyIndex_ > 0;
}

bool ImageWidget::canRedo() const {
    return historyIndex_ < history_.size() - 1;
}

void ImageWidget::resetImage() {
    if (!hasImage()) return;
    
    imageData_->reset();
    updateDisplay();
    emit imageChanged();
}

void ImageWidget::resetToOriginal() {
    if (!hasImage()) return;
    
    pushHistory();
    imageData_->reset();
    imageData_->currentImage = imageData_->originalImage.clone();
    updateDisplay();
    emit imageChanged();
}

void ImageWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, smoothTransformation_);
    
    // Fill background
    painter.fillRect(rect(), Qt::darkGray);
    
    if (!hasImage() || imageData_->displayPixmap.isNull()) {
        // Draw placeholder text
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, tr("No Image"));
        return;
    }
    
    // Calculate centered position
    int x = (width() - imageData_->displayPixmap.width()) / 2;
    int y = (height() - imageData_->displayPixmap.height()) / 2;
    
    // Draw image
    painter.drawPixmap(x, y, imageData_->displayPixmap);
    
    // Draw edit mode overlays
    switch (editMode_) {
        case ImageEditMode::Resize:
            drawResizeHandles(painter);
            break;
        case ImageEditMode::Crop:
            drawCropOverlay(painter);
            break;
        case ImageEditMode::Rotate:
            drawRotationIndicator(painter);
            break;
        default:
            break;
    }
}

void ImageWidget::mousePressEvent(QMouseEvent* event) {
    if (!hasImage()) return;
    
    if (editMode_ == ImageEditMode::Resize) {
        // Check if clicking on resize handle
        // TODO: Implement handle detection
        isResizing_ = true;
        resizeStartPos_ = event->pos();
    } else if (editMode_ == ImageEditMode::Crop && isCropping_) {
        isDragging_ = true;
        dragStartPos_ = widgetToImage(event->pos());
    }
}

void ImageWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!hasImage()) return;
    
    if (isResizing_ && editMode_ == ImageEditMode::Resize) {
        // Update resize
        // TODO: Implement resize logic
    } else if (isDragging_ && editMode_ == ImageEditMode::Crop) {
        QPoint currentPos = widgetToImage(event->pos());
        imageData_->cropRect = QRect(
            std::min(dragStartPos_.x(), currentPos.x()),
            std::min(dragStartPos_.y(), currentPos.y()),
            std::abs(currentPos.x() - dragStartPos_.x()),
            std::abs(currentPos.y() - dragStartPos_.y())
        );
        update();
    }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event)
    
    isDragging_ = false;
    isResizing_ = false;
    resizeHandle_ = -1;
}

void ImageWidget::wheelEvent(QWheelEvent* event) {
    if (!hasImage()) return;
    
    double delta = event->angleDelta().y() / 120.0;  // Standard wheel step
    double factor = 1.0 + delta * 0.1;  // 10% per step
    
    scaleImage(factor);
}

void ImageWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (hasImage()) {
        updateDisplay();
    }
}

void ImageWidget::updateDisplay() {
    if (!hasImage()) return;
    
    // Scale to fit widget while keeping aspect ratio
    QSize widgetSize = size();
    QSize imageSize(imageData_->currentImage.cols, imageData_->currentImage.rows);
    
    QSize scaledSize = imageSize.scaled(widgetSize, aspectRatioMode_);
    
    // Create display pixmap
    imageData_->displayPixmap = cvMatToQPixmap(imageData_->currentImage)
                                  .scaled(scaledSize, aspectRatioMode_, 
                                          smoothTransformation_ ? Qt::SmoothTransformation : Qt::FastTransformation);
    
    update();
}

QPixmap ImageWidget::cvMatToQPixmap(const cv::Mat& mat) {
    if (mat.empty()) return QPixmap();
    
    QImage image;
    if (mat.channels() == 3) {
        image = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
    } else if (mat.channels() == 4) {
        image = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGBA8888);
    } else if (mat.channels() == 1) {
        image = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
    } else {
        return QPixmap();
    }
    
    // Make a deep copy to ensure data persistence
    return QPixmap::fromImage(image.copy());
}

cv::Mat ImageWidget::qImageToCvMat(const QImage& image) {
    switch (image.format()) {
        case QImage::Format_RGB888:
            return cv::Mat(image.height(), image.width(), CV_8UC3, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
            return cv::Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        case QImage::Format_ARGB32_Premultiplied:
            return cv::Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        case QImage::Format_Grayscale8:
            return cv::Mat(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        default:
            // Convert to compatible format
            QImage converted = image.convertToFormat(QImage::Format_RGB888);
            return cv::Mat(converted.height(), converted.width(), CV_8UC3, 
                          const_cast<uchar*>(converted.bits()), converted.bytesPerLine());
    }
}

cv::Mat ImageWidget::rotateMat(const cv::Mat& src, int degrees) {
    if (src.empty()) return cv::Mat();
    
    cv::Point2f center(src.cols / 2.0f, src.rows / 2.0f);
    cv::Mat rotMatrix = cv::getRotationMatrix2D(center, degrees, 1.0);
    
    // Calculate new bounding box
    cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), degrees).boundingRect2f();
    rotMatrix.at<double>(0, 2) += bbox.width / 2.0 - src.cols / 2.0;
    rotMatrix.at<double>(1, 2) += bbox.height / 2.0 - src.rows / 2.0;
    
    cv::Mat rotated;
    cv::warpAffine(src, rotated, rotMatrix, bbox.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
    
    return rotated;
}

cv::Mat ImageWidget::cropMat(const cv::Mat& src, const QRect& rect) {
    if (src.empty() || rect.isEmpty()) return cv::Mat();
    
    // Ensure crop rect is within image bounds
    int x = std::max(0, rect.x());
    int y = std::max(0, rect.y());
    int width = std::min(rect.width(), src.cols - x);
    int height = std::min(rect.height(), src.rows - y);
    
    if (width <= 0 || height <= 0) return cv::Mat();
    
    cv::Rect cvRect(x, y, width, height);
    return src(cvRect).clone();
}

void ImageWidget::drawResizeHandles(QPainter& painter) {
    // TODO: Implement resize handles drawing
    Q_UNUSED(painter)
}

void ImageWidget::drawCropOverlay(QPainter& painter) {
    if (imageData_->cropRect.isEmpty()) return;
    
    QRect widgetRect = imageToWidget(imageData_->cropRect);
    
    // Draw semi-transparent overlay outside crop area
    QRegion clipRegion(rect());
    clipRegion -= QRegion(widgetRect);
    painter.setClipRegion(clipRegion);
    painter.fillRect(rect(), QColor(0, 0, 0, 128));
    painter.setClipping(false);
    
    // Draw crop rectangle
    painter.setPen(QPen(Qt::white, 2, Qt::DashLine));
    painter.drawRect(widgetRect);
    
    // Draw crop handles
    const int handleSize = 8;
    painter.setBrush(Qt::white);
    painter.setPen(Qt::black);
    
    QPoint corners[] = {
        widgetRect.topLeft(),
        widgetRect.topRight(),
        widgetRect.bottomLeft(),
        widgetRect.bottomRight(),
        QPoint(widgetRect.center().x(), widgetRect.top()),
        QPoint(widgetRect.center().x(), widgetRect.bottom()),
        QPoint(widgetRect.left(), widgetRect.center().y()),
        QPoint(widgetRect.right(), widgetRect.center().y())
    };
    
    for (const QPoint& corner : corners) {
        painter.drawRect(corner.x() - handleSize/2, corner.y() - handleSize/2, handleSize, handleSize);
    }
}

void ImageWidget::drawRotationIndicator(QPainter& painter) {
    // TODO: Implement rotation indicator drawing
    Q_UNUSED(painter)
}

QPoint ImageWidget::widgetToImage(const QPoint& widgetPos) const {
    if (!hasImage()) return QPoint();
    
    int x = (width() - imageData_->displayPixmap.width()) / 2;
    int y = (height() - imageData_->displayPixmap.height()) / 2;
    
    double scaleX = static_cast<double>(imageData_->currentImage.cols) / imageData_->displayPixmap.width();
    double scaleY = static_cast<double>(imageData_->currentImage.rows) / imageData_->displayPixmap.height();
    
    return QPoint(
        static_cast<int>((widgetPos.x() - x) * scaleX),
        static_cast<int>((widgetPos.y() - y) * scaleY)
    );
}

QRect ImageWidget::widgetToImage(const QRect& widgetRect) const {
    return QRect(widgetToImage(widgetRect.topLeft()),
                 widgetToImage(widgetRect.bottomRight()));
}

QPoint ImageWidget::imageToWidget(const QPoint& imagePos) const {
    if (!hasImage()) return QPoint();
    
    int x = (width() - imageData_->displayPixmap.width()) / 2;
    int y = (height() - imageData_->displayPixmap.height()) / 2;
    
    double scaleX = static_cast<double>(imageData_->displayPixmap.width()) / imageData_->currentImage.cols;
    double scaleY = static_cast<double>(imageData_->displayPixmap.height()) / imageData_->currentImage.rows;
    
    return QPoint(
        static_cast<int>(imagePos.x() * scaleX + x),
        static_cast<int>(imagePos.y() * scaleY + y)
    );
}

QRect ImageWidget::imageToWidget(const QRect& imageRect) const {
    return QRect(imageToWidget(imageRect.topLeft()),
                 imageToWidget(imageRect.bottomRight()));
}

void ImageWidget::pushHistory() {
    // Remove redo history if we were in the middle of the stack
    if (historyIndex_ + 1 < history_.size()) {
        history_.erase(history_.begin() + static_cast<std::ptrdiff_t>(historyIndex_) + 1, history_.end());
    }
    
    // Push current state
    history_.push_back(std::make_unique<ImageData>(*imageData_));
    historyIndex_ = history_.size() - 1;
    
    // Limit history size
    if (history_.size() > MAX_HISTORY) {
        history_.erase(history_.begin());
        if (historyIndex_ > 0) {
            historyIndex_--;
        }
    }
}

void ImageWidget::popHistory() {
    // Not used directly - undo/redo handle this
}

} // namespace gui
