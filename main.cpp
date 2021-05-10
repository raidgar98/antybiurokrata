#include <QApplication>
#include <antybiurokrata/windows/mainwindow/mainwindow.h>

#include <signal.h>	 // ::signal, ::raise
#include <boost/stacktrace.hpp>

/*

TODO

// implement scopus
curl 'https://api.elsevier.com/content/search/scopus?query=all(LISOK)&apiKey=a4a3a6dec671a73561980f44712ef25b&view=standard&start=1&httpAccept=application%2Fjson'

// do something about UI. what UI?

*/

void my_signal_handler(int signum)
{
	::signal(signum, SIG_DFL);
	std::cout << "sigfault in: " << std::this_thread::get_id() << std::endl;
	global_logger.print_stacktrace();
	// std::cout << boost::stacktrace::stacktrace() << std::endl;
	// boost::stacktrace::safe_dump_to("./backtrace.dump");
	::raise(SIGABRT);
}

int main(int argc, char* argv[])
{
	::signal(SIGSEGV, &my_signal_handler);
	::signal(SIGABRT, &my_signal_handler);

	std::locale::global(core::plPL());
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
