#pragma once

#include <QLineEdit>
#include <QListWidgetItem>
#include <QtWidgets/QMainWindow>

#include <antybiurokrata/libraries/engine/engine.h>

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
	using persons_extractor_storage_t = std::weak_ptr<core::orm::persons_extractor_t>;
	core::engine eng;

 public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

 signals:

	void send_neighbours(persons_extractor_storage_t);
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

	void collect_neighbours(persons_extractor_storage_t);
	void set_progress(const size_t);

 private:
	void normalize_text(const QString& arg1, QLineEdit& line);
	void clear_ui();

	std::list<std::shared_ptr<std::jthread>> jobs;
	Ui::MainWindow* ui;
};
