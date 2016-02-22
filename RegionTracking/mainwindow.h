#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_run_button_clicked();
    void on_open_button_clicked();
    void on_actionOpen_triggered();

private:
    Ui::MainWindow *ui;
    void get_picture();
};

#endif // MAINWINDOW_H
