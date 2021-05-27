/**
 * @file mainwindow.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contains declaration of main window
 * @version 0.1
 * @date 2021-05-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <QLineEdit>
#include <QListWidgetItem>
#include <QtWidgets/QMainWindow>

#include <antybiurokrata/libraries/engine/engine.h>
#include <antybiurokrata/windows/widget_items/widget_items.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
	class MainWindow;
}
QT_END_NAMESPACE

/**
 * @brief object representation of main window
 */
class MainWindow : public QMainWindow, private Log<MainWindow>
{
	Q_OBJECT

	using Log<MainWindow>::log;

	using relatives_t				= std::shared_ptr<int>;
	using incoming_relatives_t = typename relatives_t::weak_type;

	using report_t				= core::reports::report_t;
	using incoming_report_t = typename report_t::weak_type;

	core::engine eng;

 public:
	/** @brief default constructor */
	MainWindow(QWidget* parent = nullptr);

	/** @brief default destructor */
	~MainWindow();

 signals:

	/** @brief emitted when data is ready to be displayed */
	void send_publications(incoming_report_t);

	/** @brief emitted when next step during processing data is achieved */
	void send_progress(const size_t);

	/** @brief emitted, when disabling of user input/output is required */
	void switch_activation(const bool);

	/** @brief emitted when related persons arrive */
	void send_related(relatives_t);

 private slots:

	/** @brief keeps proper format in user input */
	void on_orcid_1_textChanged(const QString& arg1);
	/** @brief keeps proper format in user input */
	void on_orcid_2_textChanged(const QString& arg1);
	/** @brief keeps proper format in user input */
	void on_orcid_3_textChanged(const QString& arg1);
	/** @brief keeps proper format in user input */
	void on_orcid_4_textChanged(const QString& arg1);

	/** @brief handles user reequest for searching */
	void on_search_button_clicked();

	/** @brief handles user request for generating report */
	void on_generate_report_clicked();

	/** @brief handles user request for fast changing search object */
	void on_neighbours_itemDoubleClicked(QListWidgetItem* item);

	/** @brief handles user request for view of shared publications */
	void on_neighbours_itemClicked(QListWidgetItem* item);

	/** @brief handles user request for view of shared publications */
	void on_neighbours_itemChanged(QListWidgetItem* item);

	/** @brief handles user request for detailed view of selected publication */
	void on_publications_itemDoubleClicked(QListWidgetItem* item);

 public slots:

	/** @brief handles `send_neighbours` signal */
	void collect_publications(incoming_report_t);

	/** @brief handles `send_related` signal */
	void collect_related(relatives_t);

	/** @brief handles `send_progress` signal */
	void set_progress(const size_t);

	/** @brief handles `switch_activation` signal */
	void set_activation(const bool);

 private:
	/**
	 * @brief displays common publications
	 * 
	 * @param item report
	 */
	void load_publications(incoming_report_t report);

	/**
	 * @brief displays relatted persons
	 * 
	 * @param related relatives
	 */
	void load_relatives(relatives_t related);

	/**
	 * @brief recursively corrects user mistakes and provides mechanism for easy pasting ORCID
	 * 
	 * @param arg1 user input
	 * @param line line to write corrected output
	 * @param next place to propagte additional data (if nullptr [default] propagation is ignored)
	 */
	void normalize_text(const QString& arg1, QLineEdit& line, QLineEdit* next = nullptr);

	/**
	 * @brief clears both listviews
	 */
	void clear_ui();

	std::list<std::shared_ptr<std::jthread>> jobs;
	Ui::MainWindow* ui;
};
