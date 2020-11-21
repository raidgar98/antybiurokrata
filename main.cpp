#include "mainwindow.h"

#include <QApplication>

#include <sneaky_pointer/sneaky_pointer.hpp>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
