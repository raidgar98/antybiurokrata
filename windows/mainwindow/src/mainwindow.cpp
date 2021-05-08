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
	QObject::connect(this, &MainWindow::send_neighbours, this, &MainWindow::collect_neighbours);
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

void MainWindow::on_search_button_clicked()
{
	ui->neighbours->clear();
	ui->publications->clear();

	const std::string name	  = ui->name->text().toUpper().toStdString();
	const std::string surname = ui->surname->text().toUpper().toStdString();

	// bgpolsl
	core::network::bgpolsl_adapter bgadapter{};
	core::orm::persons_extractor_t bgperson_visitor{};
	core::orm::publications_extractor_t bgvisitor{bgperson_visitor};
	auto res				= bgadapter.get_person(name, surname);
	const double size = res->size();
	double i				= 0;
	for(auto& x: *res)
	{
		using namespace std::chrono_literals;
		ui->progress->setValue((i++ / size) * 100.0);
		std::this_thread::sleep_for(0.1s);
		x.accept(&bgvisitor);
	}

	// orcid
	// core::orm::persons_extractor_t orcid_person_visitor{bgperson_visitor};
	// for(auto& x: orcid_person_visitor.persons) x().publictions().clear();
	// core::orm::publications_extractor_t ovisitor{orcid_person_visitor};
	// core::network::orcid_adapter oadapter{};
	// for(const auto& person: orcid_person_visitor.persons)
	// {
	// 	auto ret = oadapter.get_person(person().orcid()());
	// 	for(auto& x: *ret) x.accept(&ovisitor);
	// }
	// ui->progress->setValue(50);

	// scopus
	// core::orm::persons_extractor_t scopus_person_visitor{bgperson_visitor};
	// for(auto& x: scopus_person_visitor.persons) x().publictions().clear();
	// core::orm::publications_extractor_t svisitor{scopus_person_visitor};
	// core::network::scopus_adapter sadapter{};
	// for(const auto& person: scopus_person_visitor.persons)
	// {
	// 	auto ret = sadapter.get_person(person().orcid()());
	// 	for(auto& x: *ret) x.accept(&svisitor);
	// }
	// ui->progress->setValue(75);

	for(const auto& p: bgperson_visitor.persons)
	{
		core::u16str label{p().name()().data()().raw + u" " + p().surname()().data()().raw + u"[ "};
		label += static_cast<core::u16str>(p().orcid()()) + u" ]";

		ui->neighbours->addItem(QString::fromStdU16String(label));

		if(p().name() == name && p().surname() == surname)
			for(const auto& x: p().publictions())
			{
				const QString year			  = QString::fromStdString(std::to_string((*x)().year()));
				const core::u16str full_name = core::u16str(u"[ ") + year.toStdU16String() + u" ] " + (*x)().raw_title;
				ui->publications->addItem(QString::fromStdU16String(full_name));
			}
	}
	ui->progress->setValue(100);
}

void MainWindow::on_generate_report_clicked() {}

void MainWindow::on_neighbours_itemChanged(QListWidgetItem* item) {}

void MainWindow::collect_neighbours(std::shared_ptr<core::orm::persons_extractor_t>) {}
