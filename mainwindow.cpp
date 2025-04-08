#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QFileInfo"
#include "QDir"
#include "QImage"
#include "QPixmap"

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

//add theme later
void MainWindow::makeMenuActions(){
    openAction = new QAction("Open", this);
    connect(openAction, &QAction::triggered ,this, &MainWindow::open);
}

void MainWindow::makeMenuBar(){

    menuBar = new QMenuBar(this);
    menu = menuBar->addMenu("File");
    menu->addAction(openAction);

    this->setMenuBar(menuBar);
}
