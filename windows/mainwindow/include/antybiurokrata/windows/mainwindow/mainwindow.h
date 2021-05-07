#pragma once

#include <QLineEdit>
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

	void on_orcid_1_textChanged(const QString& arg1);
	void on_orcid_2_textChanged(const QString& arg1);
	void on_orcid_3_textChanged(const QString& arg1);
	void on_orcid_4_textChanged(const QString& arg1);

 private:
	void normalize_text(const QString& arg1, QLineEdit& line);

	Ui::MainWindow* ui;
};
