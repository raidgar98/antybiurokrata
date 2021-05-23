#include <antybiurokrata/windows/info_dialog/info_dialog.h>
#include <antybiurokrata/windows/mainwindow/mainwindow.h>
#include <antybiurokrata/windows/account_widget_item/account_widget_item.h>

#include <antybiurokrata/libraries/bgpolsl_adapter/bgpolsl_adapter.h>
#include <antybiurokrata/libraries/orcid_adapter/orcid_adapter.h>
#include <antybiurokrata/libraries/scopus_adapter/scopus_adapter.h>

#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	qRegisterMetaType<persons_extractor_storage_t>("persons_extractor_storage_t");
	QObject::connect(this, &MainWindow::send_neighbours, this, &MainWindow::collect_neighbours);
	QObject::connect(this, &MainWindow::send_progress, this, &MainWindow::set_progress);
	// eng.on_progress.register_slot([&](const size_t progress) { this->send_progress(progress); });
	eng.on_finish.register_slot([&](std::shared_ptr<core::orm::persons_extractor_t> ptr) {
		core::dassert(ptr.get(), "person_extractior cannot be nullptr!"_u8);
		emit send_neighbours(persons_extractor_storage_t{ptr});
	});
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

void MainWindow::on_orcid_1_textChanged(const QString& arg1)
{
	this->normalize_text(arg1, *ui->orcid_1);
}

void MainWindow::on_orcid_2_textChanged(const QString& arg1)
{
	this->normalize_text(arg1, *ui->orcid_2);
}

void MainWindow::on_orcid_3_textChanged(const QString& arg1)
{
	this->normalize_text(arg1, *ui->orcid_3);
}

void MainWindow::on_orcid_4_textChanged(const QString& arg1)
{
	this->normalize_text(arg1, *ui->orcid_4);
}

void MainWindow::set_progress(const size_t p) { ui->progress->setValue(p); }

void MainWindow::on_search_button_clicked()
{
	const auto format_orcid_num = [](const QLineEdit& line) -> QString {
		std::stringstream ss;
		ss << std::setw(4) << std::setfill('0') << line.text().toStdString();
		return QString::fromStdString(ss.str());
	};

	// QListWidgetItem <- override #TODO
	// clear_ui();
	ui->neighbours->addItem("please wait...");
	ui->publications->addItem("please wait...");

	const std::string resolv_name		= ui->name->text().toUpper().toStdString();
	const std::string resolv_surname = ui->surname->text().toUpper().toStdString();
	const std::string resolv_orcid
		 = QString(format_orcid_num(*ui->orcid_1) + "-" + format_orcid_num(*ui->orcid_2) + "-"
					  + format_orcid_num(*ui->orcid_3) + "-" + format_orcid_num(*ui->orcid_4))
				 .toStdString();

	/** @note second check looks, like useless, but it's in case new tab appear */
	if(ui->tabWidget->currentIndex() == 0) /* ORCID */
		eng.start(resolv_orcid);
	else if(ui->tabWidget->currentIndex() == 1) /* Name and surname */
		eng.start(resolv_name, resolv_surname);
}

void MainWindow::on_generate_report_clicked() {}

void MainWindow::clear_ui()
{
	ui->neighbours->clear();
	ui->publications->clear();
}

void MainWindow::on_neighbours_itemChanged(QListWidgetItem* item) {}

void MainWindow::collect_neighbours(persons_extractor_storage_t bgperson_visitor)
{
	clear_ui();


	size_t cmax	 = 0;
	using coll_t = std::remove_reference<decltype(
		 (**bgperson_visitor.lock()->persons.begin())().publictions()().data())>::type;
	coll_t* coll = nullptr;

	for(const auto& _p: bgperson_visitor.lock()->persons)
	{
		auto& p = *_p;
		log.dbg() << patterns::serial::pretty_print{p} << logger::endl;
		core::u16str label{p().name()().raw + u" " + p().surname()().raw + u"[ "};
		label += static_cast<core::u16str>(p().orcid()()) + u" ]";

        account_widget_item *item = new account_widget_item();
        item->setText(QString::fromStdU16String(label));
        ui->neighbours->addItem(item);
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
			const QString year = QString::fromStdString(std::to_string((*x)().year()));
			const core::u16str full_name
				 = core::u16str(u"[ ") + year.toStdU16String() + u" ] " + (*x)().title()().raw;
			ui->publications->addItem(QString::fromStdU16String(full_name));
		}


	emit send_progress(100);
}
