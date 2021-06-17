#include <QApplication>
#include <antybiurokrata/windows/mainwindow/mainwindow.h>

#include <signal.h>	 // ::signal, ::raise
#include <boost/stacktrace.hpp>

void my_signal_handler(int signum)
{
	logger::set_current_log_level<logger::log_level::DEBUG>();
	::signal(signum, SIG_DFL);
	std::cout << "sigfault in: " << std::this_thread::get_id() << std::endl;
	global_logger.print_stacktrace();
	::raise(SIGABRT);
}

int main(int argc, char* argv[])
{
	// logger::set_current_log_level<logger::log_level::ERROR>();
	::signal(SIGSEGV, &my_signal_handler);
	::signal(SIGABRT, &my_signal_handler);

	std::locale::global(core::plPL());
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
