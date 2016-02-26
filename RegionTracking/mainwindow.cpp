#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "levelset.h"

#include <math.h>

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

    for(int i = 1; i < m_level_set.m_width - 1; i++)
    {
        for(int j = 1; j < m_level_set.m_height - 1; j++)
        {
            if (pow((pow(i-m_level_set.m_width/2,2)+pow(j-m_level_set.m_height/2,2)),0.5) < 36)
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
    for(int i = 0; i < m_level_set.m_width; i++)
    {
        m_level_set.m_u.at(i).at(0) = m_level_set.m_u.at(i).at(1);
        m_level_set.m_u.at(i).at(m_level_set.m_height - 1) = m_level_set.m_u.at(i).at(m_level_set.m_height - 2);
    }

    for(int j = 0; j < m_level_set.m_height; j++)
    {
        m_level_set.m_u.at(0).at(j) = m_level_set.m_u.at(1).at(j);
        m_level_set.m_u.at(m_level_set.m_width - 1).at(j) = m_level_set.m_u.at(m_level_set.m_width - 2).at(j);
    }

    // Four corners
    m_level_set.m_u.at(0).at(0) = m_level_set.m_u.at(1).at(1);
    m_level_set.m_u.at(0).at(m_level_set.m_height - 1) = m_level_set.m_u.at(1).at(m_level_set.m_height - 2);
    m_level_set.m_u.at(m_level_set.m_width - 1).at(0) = m_level_set.m_u.at(m_level_set.m_width - 2).at(1);
    m_level_set.m_u.at(m_level_set.m_width - 1).at(m_level_set.m_height - 1) = m_level_set.m_u.at(m_level_set.m_width-2).at(m_level_set.m_height - 2);

    float t = 0;
    int count = 0;

    while(count <= 1000)
    {
        t += m_level_set.descent_func();

        printf("%0.10f\n",t);

        m_level_set.paint_border();

        m_level_set.m_image.save("output.png");

        if(count % 100 == 0)
        {
            m_level_set.m_image.save(QString::fromStdString("./output/" + std::to_string(count) + ".png"));
        }

        /*
        pixmap = QtGui.QPixmap("./output.png")
        scaled_pixmap = pixmap.scaled(self.image_label.size(), QtCore.Qt.KeepAspectRatio)
        self.image_label.setPixmap(scaled_pixmap)
        self.image_label.setAlignment(QtCore.Qt.AlignCenter)
        print(t)
        QtGui.QApplication.processEvents()*/

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

    QGraphicsScene *scene = new QGraphicsScene();
    QPixmap pixmap(fileName);
    scene->addPixmap(pixmap);

    ui->graphicsView->fitInView(scene->itemsBoundingRect() ,Qt::KeepAspectRatio);
    ui->graphicsView->setScene(scene);

    QImage image(fileName);

    m_level_set = LevelSet(image);
}
