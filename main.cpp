#include <QApplication>
// #include <antybiurokrata/types.hpp>
#include <antybiurokrata/libraries/bgpolsl_adapter/bgpolsl_adapter.h>
#include <antybiurokrata/libraries/orm/orm.h>
// #include <antybiurokrata/windows/mainwindow/mainwindow.h>

#include <signal.h>     // ::signal, ::raise
#include <boost/stacktrace.hpp>

void my_signal_handler(int signum) {
    ::signal(signum, SIG_DFL);
	std::cout << "sigfault in: " << std::this_thread::get_id() << std::endl;
	global_logger.print_stacktrace();
	// std::cout << boost::stacktrace::stacktrace() << std::endl;
    // boost::stacktrace::safe_dump_to("./backtrace.dump");
    ::raise(SIGABRT);
}


int main(int argc, char *argv[])
{
	::signal(SIGSEGV, &my_signal_handler);
	::signal(SIGABRT, &my_signal_handler);

	std::locale::global( core::plPL() );
	// QApplication a(argc, argv);
	// MainWindow w;
	// w.show();
	core::network::bgpolsl_adapter adapter{};
	core::orm::persons_extractor_t visitor{};
	// auto result = adapter.get_person("ADRIAN", "SMAGÓR");
	auto res = adapter.get_person(argv[1], argv[2]);
	for(auto& x : *res) x.accept(&visitor);
	for(const auto& p : visitor.persons) global_logger.info() << patterns::serial::pretty_print{p} << logger::endl;
	//0000124934
	return 0;
	// return a.exec();
}
