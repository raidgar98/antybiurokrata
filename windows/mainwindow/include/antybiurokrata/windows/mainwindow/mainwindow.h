#pragma once

#include <QtWidgets/QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
	class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

 public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

 private slots:
	void on_bt_1_clicked(bool checked);

 private:
	Ui::MainWindow* ui;
};