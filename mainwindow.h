#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <editorcore.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    void makeMenuActions();
    void makeMenuBar();

    QImage mainImage;
    QImage tempImage;
    QString fileName;
    void display(QImage image);

    void open();
    void showHistogram();


    enum processType {Manual, CV, cancel};
    processType showEqualizeMethodSelectionDialog(QWidget* parent);
    void equalizeHistogram();


    struct gammaCommandInfo{
        processType procType;
        float gamma;
    };
    gammaCommandInfo showGammaCorrectionMethodSelectionDialog(QWidget* parent);
    void gammaCorrection();

    processType showThresholdMethodSelectionDialog(QWidget* parent);
    void thresholdOperation();

    EditorCore editor;

    QMenuBar *menuBar;
    QMenu *menu;

    //file
    QAction *openAction;
    QAction *saveAction;

    //Manipulate
    QAction *showHistogramAction;
    QAction *histogramEqualizationAction;
    QAction *gammaCorrectionAction;
    QAction *thresholdAction;


    QFile *imageFile;




};
#endif // MAINWINDOW_H
