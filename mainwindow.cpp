#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "editorcore.h"
#include "QFileDialog"
#include "QFileInfo"
#include "QDir"
#include "QImage"
#include "QPixmap"
#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{


    ui->setupUi(this);

    makeMenuActions();
    makeMenuBar();


    //menuBar()->setNativeMenuBar(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::open()
{
    //open file dialog
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"),
                                                    QDir::currentPath(),
                                                    tr("Image Files (*.jpg *.png *.bmp)"));

    //open image file
    imageFile = new QFile(filePath);
    QString fileName = QFileInfo(filePath).fileName();
    QImage image = QImage(filePath);
    mainImage = image;
    tempImage = mainImage;

    //init core
    editor.open(filePath);

    //set name label
    ui->fileNameLabel->setText(fileName);

    //set dimension label
    int height = image.size().height();
    int width = image.size().width();
    ui->imageSizeLabel->setText(QString("Resolution: %1x%2").arg(height).arg(width));

    //set file size label
    auto size = imageFile->size();
    ui->fileSizeLabel->setText(QString("File size: %1 bytes").arg(size));

    //set color depth label
    int colorDepth = image.depth();
    ui->imageColorDepthLabel->setText(QString("Color Depth: %1").arg(colorDepth));


    //display the image itself
    QPixmap pixmap(filePath);
    QPixmap scaledPixmap = pixmap.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->imageLabel->setPixmap(scaledPixmap);
    //ui->imageLabel->setMask(pixmap.mask());

    //resize the image so that it fits inside the screen
    ui->imageLabel->adjustSize();

}

void MainWindow::display(QImage image){
    tempImage = image;
    ui->fileNameLabel->setText(fileName);

    //set dimension label
    int height = image.size().height();
    int width = image.size().width();
    ui->imageSizeLabel->setText(QString("Resolution: %1x%2").arg(height).arg(width));

    //set file size label
    ui->fileSizeLabel->setText(QString(""));

    //set color depth label
    int colorDepth = image.depth();
    ui->imageColorDepthLabel->setText(QString("Color Depth: %1").arg(colorDepth));


    //display the image itself
    QPixmap pixmap;
    pixmap.convertFromImage(image);
    QPixmap scaledPixmap = pixmap.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->imageLabel->setPixmap(scaledPixmap);
    //ui->imageLabel->setMask(pixmap.mask());

    //resize the image so that it fits inside the screen
    ui->imageLabel->adjustSize();

}

void MainWindow::showHistogram(){
    editor.showImageHistogram(tempImage);
}


MainWindow::equalizeHistogramType MainWindow::showEqualizeMethodSelectionDialog(QWidget* parent = nullptr) {
    // Create the dialog
    QDialog dialog(parent);
    dialog.setWindowTitle("Select Method");

    // Set up layout
    QVBoxLayout layout(&dialog);

    // Create combo box
    QComboBox comboBox(&dialog);
    comboBox.addItem("Manual", Manual);
    comboBox.addItem("CV", CV);

    // Add OK button
    QPushButton okButton("OK", &dialog);

    // Arrange widgets
    layout.addWidget(&comboBox);
    layout.addWidget(&okButton);

    // Variable to store selection
    equalizeHistogramType selectedMethod = Manual;

    // Connect signals
    QObject::connect(&comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [&selectedMethod, &comboBox](int index) {
            selectedMethod = static_cast<equalizeHistogramType>(comboBox.itemData(index).toInt());
        });

    QObject::connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    // Show dialog and wait for user selection
    if (dialog.exec() == QDialog::Accepted) {
        return selectedMethod;
    }

    return Manual; // Default if dialog was canceled
}

void MainWindow::equalizeHistogram(){
    MainWindow::equalizeHistogramType type = showEqualizeMethodSelectionDialog(this);
    QImage neo;
    if(type == MainWindow::equalizeHistogramType::CV){
        neo = editor.equalizeHistogramCV(tempImage);
    }
    else{
        neo = editor.equalizeHistogramManual(tempImage);
    }
    display(neo);
}

//add theme later
void MainWindow::makeMenuActions(){
    openAction = new QAction("Open", this);
    connect(openAction, &QAction::triggered ,this, &MainWindow::open);

    showHistogramAction = new QAction("Show Histogram", this);
    connect(showHistogramAction, &QAction::triggered, this, &MainWindow::showHistogram);

    histogramEqualizationAction = new QAction("Equalize Histogram", this);
    connect(histogramEqualizationAction, &QAction::triggered, this, &MainWindow::equalizeHistogram);
}

void MainWindow::makeMenuBar(){

    menuBar = new QMenuBar(this);
    menu = menuBar->addMenu("File");
    menu->addAction(openAction);

    menu = menuBar->addMenu("Image");
    menu->addAction(showHistogramAction);
    menu->addAction(histogramEqualizationAction);

    this->setMenuBar(menuBar);
}
