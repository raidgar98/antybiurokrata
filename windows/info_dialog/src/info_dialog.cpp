#include <antybiurokrata/windows/info_dialog/info_dialog.h>
#include "ui_info_dialog.h"

info_dialog::info_dialog(const QString& to_display, QWidget* parent) : QDialog(parent), ui(new Ui::info_dialog)
{
	ui->setupUi(this);
	ui->display->setText(to_display);
}

info_dialog::~info_dialog() { delete ui; }
