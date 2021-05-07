#include <antybiurokrata/windows/info_dialog/info_dialog.h>

#include <antybiurokrata/windows/mainwindow/mainwindow.h>

#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) { ui->setupUi(this); }

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_bt_1_clicked(bool checked)
{
	std::cout << "Eluwina !!! " << checked << std::endl;
	std::unique_ptr<info_dialog> ptr{new info_dialog{this}};
	ptr->exec();
}

void MainWindow::normalize_text(const QString& arg1, QLineEdit& line)
{
	QString new_text;
	new_text.reserve(4);
	for(const auto c: arg1)
		if(c.isDigit()) new_text += c;
	line.setText(new_text);
}

void MainWindow::on_orcid_1_textChanged(const QString& arg1) { this->normalize_text(arg1, *ui->orcid_1); }

void MainWindow::on_orcid_2_textChanged(const QString& arg1) { this->normalize_text(arg1, *ui->orcid_2); }

void MainWindow::on_orcid_3_textChanged(const QString& arg1) { this->normalize_text(arg1, *ui->orcid_3); }

void MainWindow::on_orcid_4_textChanged(const QString& arg1) { this->normalize_text(arg1, *ui->orcid_4); }