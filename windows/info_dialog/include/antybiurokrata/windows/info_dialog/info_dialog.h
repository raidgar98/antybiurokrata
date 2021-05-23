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
	explicit info_dialog(const QString& to_display, QWidget* parent = nullptr);
	~info_dialog();

 private:
	Ui::info_dialog* ui;
};