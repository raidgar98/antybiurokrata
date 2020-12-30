#include <mainwindow/mainwindow.h>
#include "../ui_mainwindow.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_bt_1_clicked(bool checked)
{
    std::cout << "Eluwina !!! " << checked << std::endl;
}
