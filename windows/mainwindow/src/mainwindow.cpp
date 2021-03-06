#include <antybiurokrata/windows/info_dialog/info_dialog.h>
#include <antybiurokrata/windows/mainwindow/mainwindow.h>

#include <antybiurokrata/libraries/bgpolsl_adapter/bgpolsl_adapter.h>
#include <antybiurokrata/libraries/orcid_adapter/orcid_adapter.h>
#include <antybiurokrata/libraries/scopus_adapter/scopus_adapter.h>
#include <antybiurokrata/libraries/generator/generator.h>


#include <QKeyEvent>

#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	qRegisterMetaType<incoming_report_t>("incoming_report_t");
	qRegisterMetaType<incoming_relatives_t>("incoming_relatives_t");
	qRegisterMetaType<error_report_t>("error_report_t");

	QObject::connect(this, &MainWindow::send_publications, this, &MainWindow::collect_publications);
	QObject::connect(this, &MainWindow::send_related, this, &MainWindow::collect_related);
	QObject::connect(this, &MainWindow::send_max_progress, this, &MainWindow::set_max_progress);
	QObject::connect(this, &MainWindow::switch_activation, this, &MainWindow::set_activation);
	QObject::connect(this, &MainWindow::send_error_report, this, &MainWindow::on_error_thrown);
	QObject::connect(this,
						  &MainWindow::send_report_generation_done,
						  this,
						  &MainWindow::on_report_generation_done);
	QObject::connect(this,
						  &MainWindow::send_progress,
						  this,
						  &MainWindow::set_progress,
						  Qt::QueuedConnection);

	eng.on_calculated_progress.register_slot([&](const size_t N) { emit set_max_progress(N); });

	eng.on_finish.register_slot([&](report_t ptr) {
		core::check_nullptr{ptr};
		emit send_publications(incoming_report_t{ptr});
	});

	eng.on_collaboration_finish.register_slot([&](relatives_t ptr) {
		core::check_nullptr{ptr};
		emit send_related(incoming_relatives_t{ptr});
	});

	eng.on_progress.register_slot([&](const size_t N) { emit send_progress(N); });

	eng.on_error.register_slot([&](error_report_t report) {
		emit send_error_report(report);
		emit switch_activation(true);
		this->clear_ui();
	});
}

MainWindow::~MainWindow() { delete ui; }


bool MainWindow::handle_signal() const volatile { return this->m_handle_signals.load(); }

void MainWindow::normalize_text(const QString& arg1, QLineEdit& line, QLineEdit* next)
{
	const size_t cursor = line.cursorPosition();
	constexpr size_t max_chars{4};
	QString new_text;
	new_text.reserve(max_chars);

	size_t i = 0;
	for(const auto c: arg1)
	{
		if(c.isDigit()) new_text += c;
		if(new_text.size() == max_chars) break;
		else
			i++;
	}

	const size_t arg_size = arg1.size();
	if(arg_size > max_chars && next && cursor > max_chars)
	{
		next->setFocus();
		next->setText(arg1.right(arg_size - i - 1));
	}

	line.setText(new_text);
	line.setCursorPosition(std::min<size_t>(cursor, new_text.size()));
}

void MainWindow::on_orcid_1_textChanged(const QString& arg1)
{
	this->normalize_text(arg1, *ui->orcid_1, ui->orcid_2);
}

void MainWindow::on_orcid_2_textChanged(const QString& arg1)
{
	this->normalize_text(arg1, *ui->orcid_2, ui->orcid_3);
}

void MainWindow::on_orcid_3_textChanged(const QString& arg1)
{
	this->normalize_text(arg1, *ui->orcid_3, ui->orcid_4);
}

void MainWindow::on_orcid_4_textChanged(const QString& arg1)
{
	this->normalize_text(arg1, *ui->orcid_4);
}

void MainWindow::set_max_progress(const size_t N) { ui->progress->setMaximum(N); }

void MainWindow::set_progress(const size_t p)
{
	if(p == 1) ui->progress->setValue(ui->progress->value() + 1);
	else
		ui->progress->setValue((static_cast<double>(p) / 100.0)
									  * static_cast<double>(ui->progress->maximum()));
}


void MainWindow::on_report_generation_done()
{
	info_dialog dial{"Poprawnie wygenerowano raport!", this};
	dial.exec();
	set_activation(true);
}


void MainWindow::on_search_button_clicked()
{
	if(!handle_signal()) return;
	set_activation(false);

	const auto format_orcid_num = [](const QLineEdit& line) -> QString {
		std::stringstream ss;
		ss << std::setw(4) << std::setfill('0') << line.text().toStdString();
		return QString::fromStdString(ss.str());
	};

	ui->neighbours->addItem("please wait...");
	ui->publications->addItem("please wait...");

	emit set_progress(0);

	const std::string resolv_name		= ui->name->text().toUpper().toStdString();
	const std::string resolv_surname = ui->surname->text().toUpper().toStdString();
	const std::string resolv_orcid
		 = QString(format_orcid_num(*ui->orcid_1) + "-" + format_orcid_num(*ui->orcid_2) + "-"
					  + format_orcid_num(*ui->orcid_3) + "-" + format_orcid_num(*ui->orcid_4))
				 .toStdString();

	/** @note second check looks a bit useless, but it's in case if new tab appear */
	if(ui->tabWidget->currentIndex() == 0) /* ORCID */
		eng.start(resolv_orcid);
	else if(ui->tabWidget->currentIndex() == 1) /* Name and surname */
		eng.start(resolv_name, resolv_surname);
	else
		core::dassert(false, "not supported"_u8);
}

void MainWindow::on_generate_report_clicked()
{
	if(!handle_signal()) return;
	dassert{eng.is_last_summary_avaiable(), "no data is avaiable, cannot create report"_u8};
	switch_activation(false);

	set_max_progress(eng.get_last_summary()->size());
	QString name{"antybiurokrata_raport.xlsx"};
	if(auto* ptr = dynamic_cast<account_widget_item*>(ui->neighbours->itemAt(0, 0)))
	{
		core::check_nullptr{ptr->m_person};
		name = QString::fromStdU16String((*ptr->m_person.lock())().name()().raw).toLower()
				 + QString("_")
				 + QString::fromStdU16String((*ptr->m_person.lock())().surname()().raw).toLower()
				 + "_raport.xlsx";
	}
	m_job.reset(
		 new std::jthread{reports::generator{eng.get_last_summary(),
														 name.toStdString(),
														 [&]() { emit send_report_generation_done(); },
														 [&](const size_t N) { emit send_progress(N); }}});
}

void MainWindow::clear_ui()
{
	ui->neighbours->clear();
	ui->publications->clear();
}

void MainWindow::set_activation(const bool activate)
{
	m_handle_signals.store(activate);

	ui->search_button->setEnabled(activate);
	ui->generate_report->setEnabled(activate);
	ui->neighbours->setEnabled(activate);
	ui->publications->setEnabled(activate);
}

void MainWindow::load_publications(incoming_report_t report, const single_relative_t* filter)
{
	core::check_nullptr{report};
	ui->publications->clear();

	for(const auto& x: *(report.lock()))
	{
		if(filter)
		{
			const auto& ref  = (*(*x())().reference()())();
			const auto& pubs = (*filter)().data();
			if(std::find_if(pubs.begin(),
								 pubs.end(),
								 [&ref](const objects::shared_publication_t& c) {
									 return ref.compare((*c())()) == 0;
								 })
				== pubs.end())
				continue;
		}
		ui->publications->addItem(new publication_widget_item{x});
	}

	ui->publications->sortItems(Qt::DescendingOrder);
}


void MainWindow::collect_related(incoming_relatives_t relatives)
{
	core::check_nullptr{relatives};
	load_relatives(relatives);
}

void MainWindow::load_relatives(incoming_relatives_t related)
{
	core::check_nullptr{related};
	ui->neighbours->clear();

	for(const auto& p: *related.lock())
		ui->neighbours->addItem(new account_widget_item(p().data(), ui->neighbours));

	ui->neighbours->sortItems();
	// ui->neighbours->setCurrentRow(0);
}

void MainWindow::collect_publications(incoming_report_t report)
{
	load_publications(report);
	emit switch_activation(true);
}


void MainWindow::apply_relative_change(QListWidgetItem* item)
{
	if(!handle_signal()) return;

	core::check_nullptr{item};
	if(account_widget_item* account = dynamic_cast<account_widget_item*>(item))
	{
		auto& ref = account->m_person;
		core::check_nullptr{ref};
		const auto& pubs = (*ref.lock())().publictions();
		load_publications(eng.get_last_summary(), &pubs);
	}
	else
		core::dassert{false, "cannot cast object!"_u8};
}

void MainWindow::on_neighbours_itemClicked(QListWidgetItem* item)
{
	log.info() << "on_neighbours_itemClicked" << logger::endl;
	apply_relative_change(item);
}

void MainWindow::on_neighbours_itemChanged(QListWidgetItem* item)
{
	log.info() << "on_neighbours_itemChanged" << logger::endl;
	apply_relative_change(item);
}

void MainWindow::on_neighbours_itemDoubleClicked(QListWidgetItem* item)
{
	if(!handle_signal()) return;

	core::check_nullptr{item};
	if(account_widget_item* account = dynamic_cast<account_widget_item*>(item))
	{
		check_nullptr{account->m_person};
		const auto& person = (*account->m_person.lock())();
		const auto format	 = [](const uint16_t& num) {
			 std::stringstream ss;
			 ss << std::setw(4) << std::setfill('0') << num;
			 return QString::fromStdString(ss.str());
		};

		ui->orcid_1->setText(format(person.orcid()().identifier()[0]));
		ui->orcid_2->setText(format(person.orcid()().identifier()[1]));
		ui->orcid_3->setText(format(person.orcid()().identifier()[2]));
		ui->orcid_4->setText(format(person.orcid()().identifier()[3]));

		ui->tabWidget->setCurrentIndex(0);
	}
	else
		core::dassert{false, "cannot cast object!"_u8};
}

void MainWindow::on_publications_itemDoubleClicked(QListWidgetItem* item)
{
	if(!handle_signal()) return;

	core::check_nullptr{item};
	if(publication_widget_item* publication = dynamic_cast<publication_widget_item*>(item))
	{
		dassert{!publication->m_publication.expired(), "this publication is empty!"_u8};
		const auto& pub		 = (*publication->m_publication.lock().get())();
		const auto& pub_ref	 = (*pub.reference()())();
		const auto& pub_match = pub.matched()().data();

		auto conv = core::get_conversion_engine();
		std::stringstream ss;
		ss << "Tytuł referencyjny: " << conv.to_bytes(pub_ref.title()().raw) << std::endl
			<< std::endl;

		if(!pub_ref.polish_title()()->empty())
			ss << "Tytuł orginalny: " << conv.to_bytes(pub_ref.polish_title()().raw) << std::endl
				<< std::endl;

		ss << "Rok: " << pub_ref.year() << std::endl << std::endl;

		for(const auto& pair: pub_ref.ids()().data())
		{
			ss << conv.to_bytes(objects::detail::id_type_stringinizer::get(pair.first)) << ": "
				<< conv.to_bytes(pair.second().raw) << std::endl;
		}

		ss << std::endl;
		if(pub_match.empty()) ss << "Brak dopasowań" << std::endl;
		else
		{
			using trans_t = objects::processing_details::enums::enum_stringinizer<
				 objects::detail::match_type_translation_unit>;

			ss << "Dopsowano w ";
			if(pub_match.size() == 1) ss << "serwisie ";
			else
				ss << "serwisach: ";

			for(const auto& pair: pub_match)
				ss << conv.to_bytes(trans_t::get(pair().source()().data())) << " ";
		}

		std::unique_ptr<info_dialog> window{new info_dialog{QString::fromStdString(ss.str()), this}};
		window->setWindowTitle(QString::fromStdU16String(pub_ref.title()().raw));
		window->exec();
	}
	else
		core::dassert{false, "cannot cast object!"_u8};
}

void MainWindow::on_error_thrown(error_report_t report)
{
	check_nullptr{report};
	constexpr str_v base_error_msg{
		 R"(Podczas przetwarzania wystąpił błąd.
Jeżeli sposób naprawy nie wydaje się oczywisty skontaktuj się z autorem

e-mail: krzymoc340@student.polsl.pl
e-mail: raidgar98@onet.pl

Treść wyjątku: ")"};

	// prepare string
	std::stringstream ss;
	ss << base_error_msg << report->reason << '"' << std::endl << std::endl;
	ss << "Stos:" << std::endl << std::endl;
	ss << report->get_callstack_as_string() << std::endl;

	std::unique_ptr<info_dialog> window{new info_dialog{QString::fromStdString(ss.str()), this}};
	window->setWindowTitle(QString("Wystąpił błąd!"));
	window->exec();
}
