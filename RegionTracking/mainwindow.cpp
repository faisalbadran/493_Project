#include <Magick++.h>
using namespace Magick;

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QLabel>
#include <QPixmap>
#include <QString>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_run_button_clicked()
{

}

void MainWindow::on_open_button_clicked()
{
    get_picture();
}

void MainWindow::on_actionOpen_triggered()
{
    get_picture();
}

void MainWindow::get_picture(){
    //get a filename to open
    QString fileName = QFileDialog::getOpenFileName(this,
       tr("Open Image"), QDir::homePath().append("/Documents"), tr("Image Files (*.png *.jpg *.bmp)"));

    ui->open_label->setText(fileName);

    QGraphicsScene *scene = new QGraphicsScene();
    QPixmap pixmap(fileName);
    scene->addPixmap(pixmap);

    ui->graphicsView->fitInView(scene->itemsBoundingRect() ,Qt::KeepAspectRatio);
    ui->graphicsView->setScene(scene);

    Image image(fileName.toStdString());
    printf(image.magick().c_str());
}
