#pragma once

#include <QLineEdit>
#include <QListWidgetItem>
#include <QtWidgets/QMainWindow>

#include <antybiurokrata/libraries/orm/orm.h>

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

signals:

	void send_neighbours(std::shared_ptr<core::orm::persons_extractor_t>);

private slots:

	void on_orcid_1_textChanged(const QString& arg1);
	void on_orcid_2_textChanged(const QString& arg1);
	void on_orcid_3_textChanged(const QString& arg1);
	void on_orcid_4_textChanged(const QString& arg1);

	void on_search_button_clicked();
	void on_generate_report_clicked();
	void on_neighbours_itemChanged(QListWidgetItem* item);

public slots:

	void collect_neighbours(std::shared_ptr<core::orm::persons_extractor_t>);

 private:
	void normalize_text(const QString& arg1, QLineEdit& line);

	Ui::MainWindow* ui;
};
