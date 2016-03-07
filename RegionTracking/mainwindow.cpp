#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "levelset.h"

#include <math.h>

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QImage>
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

    // If outside region set to -1
    // If inside region set to +1

    for(int i = 1; i < m_level_set.m_width-1; i++)
    {
        for(int j = 1; j < m_level_set.m_height-1; j++)
        {
            if (pow((pow(i-m_level_set.m_width/2,2)+pow(j-m_level_set.m_height/2,2)),0.5) < 25)
            {
                m_level_set.m_u.at(i).at(j) = 1.0;
            }
            else
            {
                m_level_set.m_u.at(i).at(j)= -1.0;
            }
        }
    }

    // Mirror image the border
    m_level_set.mirror_u();

    float t = 0;
    int count = 0;

    while(count <= 100000)
    {
        t += m_level_set.descent_func();

        m_level_set.paint_border();

        if(count % 100 == 0)
        {
            m_level_set.m_image.save(QString::fromStdString("./output/" + std::to_string(count) + ".png"));
        }


        QPixmap pixmap(QPixmap::fromImage(m_level_set.m_image));
        QGraphicsScene* scene = new QGraphicsScene;
        scene->addPixmap(pixmap);

        ui->graphicsView->fitInView(scene->itemsBoundingRect() ,Qt::KeepAspectRatio);
        ui->graphicsView->setScene(scene);

        ui->graphicsView->repaint();
        qApp->processEvents();

        delete scene;

        count += 1;
    }
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
       tr("Open Image"), QDir::homePath().append("/Documents"), tr("Image Files (*.png *.jpg *.bmp *.jpeg)"));

    ui->open_label->setText(fileName);

    QImage image(fileName);

    QPixmap pixmap(QPixmap::fromImage(image));
    QGraphicsScene* scene = new QGraphicsScene;
    scene->addPixmap(pixmap);

    ui->graphicsView->fitInView(scene->itemsBoundingRect() ,Qt::KeepAspectRatio);
    ui->graphicsView->setScene(scene);

    m_level_set = LevelSet(image);
}
