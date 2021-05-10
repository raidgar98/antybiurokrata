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

class MainWindow : public QMainWindow, private Log<MainWindow>
{
	Q_OBJECT

	using Log<MainWindow>::log;

 public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

 signals:

	void send_neighbours(QSharedPointer<core::orm::persons_extractor_t>);
	void send_progress(const size_t);

 private slots:

	void on_orcid_1_textChanged(const QString& arg1);
	void on_orcid_2_textChanged(const QString& arg1);
	void on_orcid_3_textChanged(const QString& arg1);
	void on_orcid_4_textChanged(const QString& arg1);

	void on_search_button_clicked();
	void on_generate_report_clicked();
	void on_neighbours_itemChanged(QListWidgetItem* item);

 public slots:

	void collect_neighbours(QSharedPointer<core::orm::persons_extractor_t>);
	void set_progress(const size_t);

 private:
	void normalize_text(const QString& arg1, QLineEdit& line);
	void clear_ui();

	std::list<std::shared_ptr<std::jthread>> jobs;
	Ui::MainWindow* ui;
};
