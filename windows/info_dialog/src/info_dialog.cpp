#include <antybiurokrata/windows/info_dialog/info_dialog.h>
#include "ui_info_dialog.h"

info_dialog::info_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::info_dialog)
{
    ui->setupUi(this);
}

info_dialog::~info_dialog()
{
    delete ui;
}
