#include <antybiurokrata/windows/info_dialog/info_dialog.h>
#include <antybiurokrata/windows/mainwindow/mainwindow.h>

#include <antybiurokrata/libraries/bgpolsl_adapter/bgpolsl_adapter.h>
#include <antybiurokrata/libraries/orcid_adapter/orcid_adapter.h>
#include <antybiurokrata/libraries/scopus_adapter/scopus_adapter.h>

#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	qRegisterMetaType<QSharedPointer<core::orm::persons_extractor_t>>("QSharedPointer<core::orm::persons_extractor_t>");
	QObject::connect(this, &MainWindow::send_neighbours, this, &MainWindow::collect_neighbours);
	QObject::connect(this, &MainWindow::send_progress, this, &MainWindow::set_progress);
}

MainWindow::~MainWindow() { delete ui; }

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

void MainWindow::set_progress(const size_t p) { ui->progress->setValue(p); }

void MainWindow::on_search_button_clicked()
{
	const auto format_orcid_num = [](const QLineEdit& line) -> QString {
		std::stringstream ss;
		ss << std::setw(4) << std::setfill('0') << line.text().toStdString();
		return QString::fromStdString(ss.str());
	};


	// clear_ui();
	ui->neighbours->addItem("please wait...");
	ui->publications->addItem("please wait...");

	const std::string resolv_name		= ui->name->text().toUpper().toStdString();
	const std::string resolv_surname = ui->surname->text().toUpper().toStdString();
	const std::string resolv_orcid	= QString(format_orcid_num(*ui->orcid_1) + "-" + format_orcid_num(*ui->orcid_2) + "-"
															 + format_orcid_num(*ui->orcid_3) + "-" + format_orcid_num(*ui->orcid_4))
													 .toStdString();

	// log << "orcid: " << resolv_orcid << logger::endl;
	// if((resolv_name.empty() || resolv_surname.empty()) && resolv_orcid.empty()) return;
	// return;

	jobs.emplace_back(std::make_shared<std::jthread>(
		 [&](const core::str name, const core::str surname) {
			 // bgpolsl
			 core::network::bgpolsl_adapter bgadapter{};
			 QSharedPointer<core::orm::persons_extractor_t> sh_bgperson_visitor{new core::orm::persons_extractor_t{}};
			 auto& bgperson_visitor = *sh_bgperson_visitor;
			 core::orm::publications_extractor_t bgvisitor{bgperson_visitor};
			 auto res	 = bgadapter.get_person(name, surname);
			 double size = res->size() * 3;
			 double i	 = 0;
			 for(auto& x: *res)
			 {
				 using namespace std::chrono_literals;
				 emit set_progress((i++ / size) * 100.0);
				 // std::this_thread::sleep_for(0.1s);
				 x.accept(&bgvisitor);
			 }

			 std::initializer_list<std::jthread>{
				  std::jthread{[&] {
					  // orcid
					  core::orm::persons_extractor_t orcid_person_visitor;
					  core::orm::persons_extractor_t::shallow_copy_persons(bgperson_visitor, orcid_person_visitor);

					  for(auto& x: orcid_person_visitor.persons) (*x)().publictions()()->clear();
					  core::orm::publications_extractor_t ovisitor{orcid_person_visitor};
					  core::network::orcid_adapter oadapter{};
					  for(const auto& _person: orcid_person_visitor.persons)
					  {
						  auto& person = *_person;
						  auto ret		= oadapter.get_person(person().orcid()());
						  emit set_progress((i++ / size) * 100.0);
						  for(auto& x: *ret) x.accept(&ovisitor);
					  }
				  }},
				  std::jthread{[&] {
					  // scopus
					  core::orm::persons_extractor_t scopus_person_visitor;
					  core::orm::persons_extractor_t::shallow_copy_persons(bgperson_visitor, scopus_person_visitor);

					  for(auto& x: scopus_person_visitor.persons) (*x)().publictions()()->clear();
					  core::orm::publications_extractor_t svisitor{scopus_person_visitor};
					  core::network::scopus_adapter sadapter{};
					  for(const auto& _person: scopus_person_visitor.persons)
					  {
						  auto& person = *_person;
						  auto ret		= sadapter.get_person(person().orcid()());
						  emit set_progress((i++ / size) * 100.0);
						  for(auto& x: *ret) x.accept(&svisitor);
					  }
				  }}};

			 emit send_neighbours(sh_bgperson_visitor);
		 },
		 resolv_name,
		 resolv_surname));
}

void MainWindow::on_generate_report_clicked() {}

void MainWindow::clear_ui()
{
	ui->neighbours->clear();
	ui->publications->clear();
}

void MainWindow::on_neighbours_itemChanged(QListWidgetItem* item) {}

void MainWindow::collect_neighbours(QSharedPointer<core::orm::persons_extractor_t> bgperson_visitor)
{
	clear_ui();

	size_t cmax	 = 0;
	using coll_t = std::remove_reference<decltype((**bgperson_visitor->persons.begin())().publictions()().data())>::type;
	coll_t* coll = nullptr;

	for(const auto& _p: bgperson_visitor->persons)
	{
		auto& p = *_p;
		core::u16str label{p().name()().raw + u" " + p().surname()().raw + u"[ "};
		label += static_cast<core::u16str>(p().orcid()()) + u" ]";

		ui->neighbours->addItem(QString::fromStdU16String(label));
		const size_t mmm = p().publictions()()->size();
		if(mmm > cmax)
		{
			cmax = mmm;
			coll = &(p().publictions()().data());
		}
	}

	if(coll)
		for(const auto& x: *coll)
		{
			const QString year			  = QString::fromStdString(std::to_string((*x)().year()));
			const core::u16str full_name = core::u16str(u"[ ") + year.toStdU16String() + u" ] " + (*x)().title()().raw;
			ui->publications->addItem(QString::fromStdU16String(full_name));
		}

	emit send_progress(100);
}
