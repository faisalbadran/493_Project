#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "levelset.h"

#include <algorithm>
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
    // Create output firectory
    unsigned last = m_filename.find_last_of('/');

    m_output_filename = m_filename.substr(0,last);
    m_output_filename = m_output_filename + "/output.";

    // Assign coefficients

    m_level_set.m_coef_length = stof(ui->length_edit->text().toStdString());
    m_level_set.m_coef_mean_inside = stof(ui->mean_inside_edit->text().toStdString());
    m_level_set.m_coef_mean_outside = stof(ui->mean_outside_edit->text().toStdString());
    m_level_set.m_coef_variance_inside = stof(ui->variance_inside_edit->text().toStdString());
    m_level_set.m_coef_variance_outside = stof(ui->variance_outside_edit->text().toStdString());
    m_level_set.m_coef_area = stof(ui->area_edit->text().toStdString());
    m_level_set.m_coef_com = stof(ui->com_edit->text().toStdString());
    m_level_set.m_coef_pixel = stof(ui->pixel_edit->text().toStdString());
    m_level_set.m_iterations = stoi(ui->iterations_edit->text().toStdString());

    // If outside region set to -1
    // If inside region set to +1
    // Magical color is: RGB = (255,0,23)

    for(int i = 1; i < m_level_set.m_width-1; i++)
    {
        for(int j = 1; j < m_level_set.m_height-1; j++)
        {
            QRgb pixel = m_level_set.m_image.pixel(i-1, j-1);
            if (qRed(pixel) == 255 && qGreen(pixel) == 0 && qBlue(pixel) == 0)
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

    m_picture_num = m_picture_num+1;
    std::string name = (m_filename + std::to_string(m_picture_num) + m_file_extension);

    // Get parameters from first image and paint border
    // This is done because the loaded image is
    // the first image with the region coloured in
    m_level_set.m_image_master = QImage(QString::fromStdString(name)).convertToFormat(QImage::Format_RGB32);
    m_level_set.calculate_parameters();
    m_level_set.paint_border();
    m_level_set.m_image.save(QString::fromStdString(m_output_filename + std::to_string(m_picture_num) + m_file_extension));
    m_picture_num = m_picture_num+1;
    name = (m_filename + std::to_string(m_picture_num) + m_file_extension);

    // Loop while there are still more pictures
    while(FILE *file = fopen(name.c_str(), "r"))
    {
        m_level_set.m_image_master = QImage(QString::fromStdString(name)).convertToFormat(QImage::Format_RGB32);

        // t is a measure of how fast the functional is moving
        // Current scheme is to do 100 iterations
        t = 0;
        for(int i = 0; i < m_level_set.m_iterations; i++)
        {
            t += m_level_set.descent_func();
        }

        // Refresh parameters
        m_level_set.calculate_parameters();

        m_level_set.paint_border();
        m_level_set.m_image.save(QString::fromStdString(m_output_filename + std::to_string(m_picture_num) + m_file_extension));

        // reset u to +/- 1
        //m_level_set.unitize_u();

        QPixmap pixmap(QPixmap::fromImage(m_level_set.m_image));
        QGraphicsScene* scene = new QGraphicsScene;
        scene->addPixmap(pixmap);

        ui->graphicsView->fitInView(scene->itemsBoundingRect() ,Qt::KeepAspectRatio);
        ui->graphicsView->setScene(scene);

        ui->graphicsView->repaint();
        qApp->processEvents();

        // Close file and prepare new file name
        fclose(file);

        m_picture_num += 1;
        name = (m_filename + std::to_string(m_picture_num) + m_file_extension);
    }

    // Set the new working directory to the output directory
    m_filename = m_output_filename;
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

    // Remove the number and extension from picture
    // Assume that the first picture starts with 1
    // ~/Documents/pic.1.bmp -> ~/Documents/pic
    m_filename = fileName.toStdString();
    unsigned first = 0;
    unsigned last = 0;

    for(std::string::size_type i = 0; i < m_filename.size(); ++i) {
        if(m_filename.at(i) == '.')
        {
            first = last;
            last = i;
        }
    }

    m_picture_num = stoi(m_filename.substr(first+1,last-first-1));
    m_file_extension = m_filename.substr(last,std::string::npos);
    m_filename = m_filename.substr(0,first+1);

    QImage image(fileName);
    image = image.convertToFormat(QImage::Format_RGB32);

    QPixmap pixmap(QPixmap::fromImage(image));
    QGraphicsScene* scene = new QGraphicsScene;
    scene->addPixmap(pixmap);

    ui->graphicsView->fitInView(scene->itemsBoundingRect() ,Qt::KeepAspectRatio);
    ui->graphicsView->setScene(scene);

    m_level_set = LevelSet(image);
}

void MainWindow::on_previous_button_clicked()
{
    m_picture_num = std::max(m_picture_num-1,0);

    // Load Image
    QImage image(QString::fromStdString(m_filename + std::to_string(m_picture_num) + m_file_extension));

    QPixmap pixmap(QPixmap::fromImage(image));
    QGraphicsScene* scene = new QGraphicsScene;
    scene->addPixmap(pixmap);

    ui->graphicsView->fitInView(scene->itemsBoundingRect() ,Qt::KeepAspectRatio);
    ui->graphicsView->setScene(scene);
}

void MainWindow::on_next_button_clicked()
{
    m_picture_num = m_picture_num+1;

    // Check if picture exists
    std::string name = (m_filename + std::to_string(m_picture_num) + m_file_extension);
    if (FILE *file = fopen(name.c_str(), "r"))
    {
        fclose(file);
    }
    else
    {
        m_picture_num = m_picture_num-1;
    }

    QImage image(QString::fromStdString(m_filename + std::to_string(m_picture_num) + m_file_extension));

    QPixmap pixmap(QPixmap::fromImage(image));
    QGraphicsScene* scene = new QGraphicsScene;
    scene->addPixmap(pixmap);

    ui->graphicsView->fitInView(scene->itemsBoundingRect() ,Qt::KeepAspectRatio);
    ui->graphicsView->setScene(scene);
}
