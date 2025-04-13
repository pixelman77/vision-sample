#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <QFile>

#ifndef EDITORCORE_H
#define EDITORCORE_H


class EditorCore
{
public:
    EditorCore();

    void open(QString path);
    bool isReady();


    QImage MatToQImage(const cv::Mat &mat);
    cv::Mat QImageToMat(const QImage &qImage);
    void showImageHistogram(QImage image);

    QImage equalizeHistogramManual(QImage image);
    QImage equalizeHistogramCV(QImage image);

    bool ready = false;

    QString path;
    cv::Mat img;
};

#endif // EDITORCORE_H
