/**
 * @file info_dialog.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief constains declaration of info dialog window
 * @version 0.1
 * @date 2021-05-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <QtWidgets/QDialog>

namespace Ui
{
	class info_dialog;
}

/**
 * @brief fancier message box
 */
class info_dialog : public QDialog
{
	Q_OBJECT

 public:
	explicit info_dialog(const QString& to_display, QWidget* parent = nullptr);
	~info_dialog();

 private:
	Ui::info_dialog* ui;
};