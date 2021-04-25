#include <QApplication>
#include <antybiurokrata/types.hpp>
#include <antybiurokrata/windows/mainwindow/mainwindow.h>

int main(int argc, char *argv[])
{
	std::locale::global( std::locale{ core::polish_locale.data() } );
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
