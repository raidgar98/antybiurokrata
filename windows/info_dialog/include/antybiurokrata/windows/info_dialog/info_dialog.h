#pragma once

#include <QtWidgets/QDialog>

namespace Ui
{
	class info_dialog;
}

class info_dialog : public QDialog
{
	Q_OBJECT

public:
	explicit info_dialog(QWidget *parent = nullptr);
	~info_dialog();

private:
	Ui::info_dialog *ui;
};