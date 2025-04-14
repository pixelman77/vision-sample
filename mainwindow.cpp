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
#include <QDoubleSpinBox>
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


MainWindow::processType MainWindow::showEqualizeMethodSelectionDialog(QWidget* parent = nullptr) {
    // Create the dialog
    QDialog dialog(parent);
    dialog.setWindowTitle("Select Method");

    // Set up layout
    QVBoxLayout layout(&dialog);

    // Create combo box
    QComboBox comboBox(&dialog);
    comboBox.addItem("Manual", MainWindow::processType::Manual);
    comboBox.addItem("CV", MainWindow::processType::CV);

    // Add OK button
    QPushButton okButton("OK", &dialog);

    // Arrange widgets
    layout.addWidget(&comboBox);
    layout.addWidget(&okButton);

    // Variable to store selection
    processType selectedMethod = MainWindow::processType::Manual;

    // Connect signals
    QObject::connect(&comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [&selectedMethod, &comboBox](int index) {
            selectedMethod = static_cast<processType>(comboBox.itemData(index).toInt());
        });

    QObject::connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    // Show dialog and wait for user selection
    if (dialog.exec() == QDialog::Accepted) {
        return selectedMethod;
    }

    return cancel; // Default if dialog was canceled
}

void MainWindow::equalizeHistogram(){
    MainWindow::processType type = showEqualizeMethodSelectionDialog(this);
    if(type == MainWindow::processType::cancel){
        return;
    }
    QImage neo;
    if(type == MainWindow::processType::CV){
        neo = editor.equalizeHistogramCV(tempImage);
    }
    else{
        neo = editor.equalizeHistogramManual(tempImage);
    }
    display(neo);
}


MainWindow::gammaCommandInfo MainWindow::showGammaCorrectionMethodSelectionDialog(QWidget* parent = nullptr) {
    MainWindow::gammaCommandInfo command;
    command.procType = MainWindow::processType::cancel;
    command.gamma = 1.0;
    QDialog dialog(parent);
    dialog.setWindowTitle("Method Selection");

    QVBoxLayout mainLayout(&dialog);

    // Method selection combo box
    QHBoxLayout methodLayout;
    QLabel methodLabel("Method:");
    QComboBox methodCombo;
    methodCombo.addItem("Manual", Manual);
    //methodCombo.addItem("CV", CV);
    methodLayout.addWidget(&methodLabel);
    methodLayout.addWidget(&methodCombo);


    QHBoxLayout gammaLayout;
    QLabel gammaLabel("gamma:");
    QDoubleSpinBox gammaSpinBox;
    gammaSpinBox.setRange(0.1, 10.0);
    gammaSpinBox.setValue(1.0);
    gammaSpinBox.setSingleStep(0.1);
    gammaSpinBox.setDecimals(1);



    gammaLayout.addWidget(&gammaLabel);

    gammaLayout.addWidget(&gammaSpinBox);


    // OK button
    QPushButton okButton("OK");

    // Arrange all widgets
    mainLayout.addLayout(&methodLayout);
    mainLayout.addLayout(&gammaLayout);
    mainLayout.addWidget(&okButton);

    // Connect signals
    QObject::connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        command.procType = static_cast<MainWindow::processType>(methodCombo.currentData().toInt());
        command.gamma = static_cast<float>(gammaSpinBox.value());
    }

    return command;

}

void MainWindow::gammaCorrection(){
    MainWindow::gammaCommandInfo command = showGammaCorrectionMethodSelectionDialog(this);
    if(command.procType == MainWindow::processType::cancel){
        return;
    }
    QImage neo;
    if(command.procType == MainWindow::processType::CV){
        //neo = editor.correctGammaCV(tempImage, command.gamma);
    }
    else{
        neo = editor.correctGammaManual(tempImage, command.gamma);
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

    gammaCorrectionAction = new QAction("Gamma Correction", this);
    connect(gammaCorrectionAction, &QAction::triggered, this, &MainWindow::gammaCorrection);
}

void MainWindow::makeMenuBar(){

    menuBar = new QMenuBar(this);
    menu = menuBar->addMenu("File");
    menu->addAction(openAction);

    menu = menuBar->addMenu("Image");
    menu->addAction(showHistogramAction);
    menu->addAction(histogramEqualizationAction);
    menu->addAction(gammaCorrectionAction);

    this->setMenuBar(menuBar);
}
