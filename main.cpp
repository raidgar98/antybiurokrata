#include <QApplication>
// #include <antybiurokrata/types.hpp>
#include <antybiurokrata/libraries/bgpolsl_adapter/bgpolsl_adapter.h>
// #include <antybiurokrata/windows/mainwindow/mainwindow.h>

int main(int argc, char *argv[])
{
	std::locale::global( std::locale{ core::polish_locale.data() } );
	// QApplication a(argc, argv);
	// MainWindow w;
	// w.show();
	core::network::bgpolsl_adapter adapter{};
	auto result = adapter.get_person("ADRIAN", "SMAGÓR");
	result->begin()->print();
	return 0;
	// return a.exec();
}
