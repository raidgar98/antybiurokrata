#include "mainwindow.h"

#include <QApplication>

#include <sneaky_pointer/sneaky_pointer.hpp>

int main(int argc, char *argv[])
{
	sneaky_pointer<std::string> p{ new int(2) };
	delete p.get_pointer();
	p.set_pointer(nullptr);

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
