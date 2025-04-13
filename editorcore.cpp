#include "editorcore.h"
#include "opencv2/opencv.hpp"
#include <QDebug>
#include <vector>
#include <opencv2/opencv.hpp>
#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QtMath>
#include <QVector>

EditorCore::EditorCore()
{
}

bool EditorCore::isReady(){
    if(!ready){
        qDebug() << "Tried to edit with no image present at:" << __FILE__ << "Line:" << __LINE__ << "Function:" << __FUNCTION__;
    }
    return ready;
}

void EditorCore::open(QString path)
{
    this->path = path;
    this->img = cv::imread(path.toStdString());
    ready = true;
}

QImage EditorCore::MatToQImage(const cv::Mat &mat) {
    if (mat.empty()) {
        return QImage();
    }

    // Handle 8-bit grayscale (1 channel)
    if (mat.type() == CV_8UC1) {
        QImage image(mat.data,
                    mat.cols,
                    mat.rows,
                    static_cast<int>(mat.step),
                    QImage::Format_Grayscale8);
        return image.copy(); // Deep copy to detach from OpenCV memory
    }
    // Handle 8-bit BGR (3 channels)
    else if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB); // Convert BGR to RGB
        QImage image(rgb.data,
                    rgb.cols,
                    rgb.rows,
                    static_cast<int>(rgb.step),
                    QImage::Format_RGB888);
        return image.copy();
    }
    // Handle 8-bit BGRA (4 channels)
    else if (mat.type() == CV_8UC4) {
        cv::Mat rgba;
        cv::cvtColor(mat, rgba, cv::COLOR_BGRA2RGBA); // Convert BGRA to RGBA
        QImage image(rgba.data,
                    rgba.cols,
                    rgba.rows,
                    static_cast<int>(rgba.step),
                    QImage::Format_RGBA8888);
        return image.copy();
    }
    // Handle 16-bit grayscale
    else if (mat.type() == CV_16UC1) {
        cv::Mat temp;
        mat.convertTo(temp, CV_8UC1, 1.0/256.0); // Scale down to 8-bit
        QImage image(temp.data,
                    temp.cols,
                    temp.rows,
                    static_cast<int>(temp.step),
                    QImage::Format_Grayscale8);
        return image.copy();
    }
    // Handle floating point matrices
    else if (mat.type() == CV_32FC1 || mat.type() == CV_64FC1) {
        cv::Mat temp;
        double minVal, maxVal;
        cv::minMaxLoc(mat, &minVal, &maxVal);
        mat.convertTo(temp, CV_8UC1, 255.0/(maxVal-minVal), -minVal*255.0/(maxVal-minVal));
        QImage image(temp.data,
                    temp.cols,
                    temp.rows,
                    static_cast<int>(temp.step),
                    QImage::Format_Grayscale8);
        return image.copy();
    }

    // Unsupported format - return null image
    return QImage();
}

cv::Mat EditorCore::QImageToMat(const QImage &qImage) {
    if (qImage.isNull()) {
        return cv::Mat();
    }

    // Convert to RGB888 if not already in a compatible format
    QImage image = qImage;
    switch (image.format()) {
        case QImage::Format_Indexed8:
        case QImage::Format_Grayscale8: {
            // Handle 8-bit grayscale
            return cv::Mat(image.height(),
                         image.width(),
                         CV_8UC1,
                         const_cast<uchar*>(image.bits()),
                         static_cast<size_t>(image.bytesPerLine()))
                         .clone();
        }

        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied: {
            // Convert 32-bit formats to 24-bit RGB
            image = image.convertToFormat(QImage::Format_RGB888);
            // Fall through to RGB888 handling
        }
        // Intentional fallthrough - braces don't prevent this
        case QImage::Format_RGB888: {
            // Convert RGB to BGR
            cv::Mat mat(image.height(),
                       image.width(),
                       CV_8UC3,
                       const_cast<uchar*>(image.bits()),
                       static_cast<size_t>(image.bytesPerLine()));

            cv::Mat bgr;
            cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR);
            return bgr;
        }

        case QImage::Format_RGBA8888:
        case QImage::Format_RGBA8888_Premultiplied: {
            // Convert RGBA to BGRA
            cv::Mat mat(image.height(),
                       image.width(),
                       CV_8UC4,
                       const_cast<uchar*>(image.bits()),
                       static_cast<size_t>(image.bytesPerLine()));

            cv::Mat bgra;
            cv::cvtColor(mat, bgra, cv::COLOR_RGBA2BGRA);
            return bgra;
        }

        default: {
            // Convert unsupported formats to RGB888
            image = image.convertToFormat(QImage::Format_RGB888);
            cv::Mat mat(image.height(),
                       image.width(),
                       CV_8UC3,
                       const_cast<uchar*>(image.bits()),
                       static_cast<size_t>(image.bytesPerLine()));

            cv::Mat bgr;
            cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR);
            return bgr;
        }
    }
}


void EditorCore::showImageHistogram(QImage image){
    if(!isReady()){ return; }
    cv::Mat cImage = QImageToMat(image);
    // Convert to grayscale if needed
    cv::Mat gray;
    if (cImage.channels() > 1) {
        cv::cvtColor(cImage, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = cImage.clone();
    }

    // Calculate histogram
    const int histSize = 256;
    const int markerInterval = 30; // Show a marker every 30 values
    float range[] = {0, 256};
    const float* histRange = {range};
    cv::Mat hist;
    cv::calcHist(&gray, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

    // Create visualization
    const int histWidth = 512;
    const int histHeight = 400;
    const int bottomMargin = 30; // Space for markers
    QImage histImage(histWidth, histHeight + bottomMargin, QImage::Format_ARGB32);
    histImage.fill(Qt::white);

    // Normalize and draw histogram
    cv::normalize(hist, hist, 0, histHeight, cv::NORM_MINMAX);
    QPainter painter(&histImage);
    painter.setPen(QPen(Qt::red, 2));

    const int binWidth = histWidth / histSize;
    for (int i = 0; i < histSize; ++i) {
        int height = cvRound(hist.at<float>(i));
        painter.drawLine(i * binWidth, histHeight,
                        i * binWidth, histHeight - height);
    }

    // Draw markers and numbers
    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    for (int i = 0; i < histSize; i += markerInterval) {
        int x = i * binWidth;
        // Draw tick mark (unchanged)
        painter.drawLine(x, histHeight, x, histHeight + 5);
        // Draw number - MOVED UP BY 5 PIXELS (changed y from +20 to +15)
        painter.drawText(x - 10, histHeight + 15, 20, 20,
                        Qt::AlignCenter, QString::number(i));
    }

    // Create popup window
    QDialog* popup = new QDialog();
    popup->setWindowTitle("Brightness Histogram with Markers");
    popup->setFixedSize(histWidth + 40, histHeight + bottomMargin + 60);

    QLabel* imageLabel = new QLabel(popup);
    imageLabel->setPixmap(QPixmap::fromImage(histImage));

    // Add axis labels
    QLabel* xAxisLabel = new QLabel("Pixel Intensity Values", popup);
    xAxisLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout* mainLayout = new QVBoxLayout(popup);
    mainLayout->addWidget(imageLabel);
    mainLayout->addWidget(xAxisLabel);

    popup->setLayout(mainLayout);
    popup->exec();
}


QImage EditorCore::equalizeHistogramCV(QImage image){
    if (image.isNull()) {
        return QImage();
    }

    // Convert QImage to cv::Mat
    cv::Mat cvImage = QImageToMat(image);

    // Convert to grayscale if needed
    cv::Mat grayImage;
    if (cvImage.channels() == 3) {
        cv::cvtColor(cvImage, grayImage, cv::COLOR_RGB2GRAY);
    } else if (cvImage.channels() == 4) {
        cv::cvtColor(cvImage, grayImage, cv::COLOR_RGBA2GRAY);
    } else {
        grayImage = cvImage.clone();
    }

    // Equalize histogram using OpenCV
    cv::Mat equalizedImage;
    cv::equalizeHist(grayImage, equalizedImage);

    // Convert back to QImage
    return MatToQImage(equalizedImage);
}

QImage EditorCore::equalizeHistogramManual(QImage image){
    if (image.isNull())
        return QImage();

    // Convert to grayscale if needed (for color images)
    if (image.format() != QImage::Format_Grayscale8) {
        image = image.convertToFormat(QImage::Format_Grayscale8);
    }

    const int width = image.width();
    const int height = image.height();
    const int pixelCount = width * height;

    //Calculate histogram
    QVector<int> histogram(256, 0);
    for (int y = 0; y < height; ++y) {
        const uchar* scanLine = image.constScanLine(y);
        for (int x = 0; x < width; ++x) {
            histogram[scanLine[x]]++;
        }
    }

    //Calculate cumulative distribution function (CDF)
    QVector<int> cdf(256, 0);
    cdf[0] = histogram[0];
    for (int i = 1; i < 256; ++i) {
        cdf[i] = cdf[i-1] + histogram[i];
    }

    //Find minimum non-zero CDF value
    int cdfMin = 0;
    for (int i = 0; i < 256; ++i) {
        if (cdf[i] != 0) {
            cdfMin = cdf[i];
            break;
        }
    }

    //Apply histogram equalization
    QImage equalized(width, height, QImage::Format_Grayscale8);
    for (int y = 0; y < height; ++y) {
        const uchar* srcLine = image.constScanLine(y);
        uchar* dstLine = equalized.scanLine(y);

        for (int x = 0; x < width; ++x) {
            int pixelValue = srcLine[x];
            int newValue = qRound(255.0 * (cdf[pixelValue] - cdfMin) / (pixelCount - cdfMin));
            dstLine[x] = qBound(0, newValue, 255);
        }
    }

    return equalized;
}

