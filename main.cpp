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

	// std::locale::global(core::plPL());
	// QApplication a(argc, argv);
	// MainWindow w;
	// w.show();
	// return a.exec();

	core::network::bgpolsl_adapter bgadapter{};
	std::shared_ptr<core::orm::persons_extractor_t> sh_bgperson_visitor{
		new core::orm::persons_extractor_t{}};
	auto& bgperson_visitor = *sh_bgperson_visitor;
	core::orm::publications_extractor_t bgvisitor{bgperson_visitor};
	auto res	 = bgadapter.get_person(argv[1], argv[2]);
	double size = res->size() * 3;
	double i	 = 0;
	for(auto& x: *res)
	{
		using namespace std::chrono_literals;
		// emit set_progress((i++ / size) * 100.0);
		// std::this_thread::sleep_for(0.1s);
		x.accept(&bgvisitor);
	}

	for(const auto& x : bgperson_visitor.persons) std::cout << patterns::serial::pretty_print{ *x } << std::endl;

	return 0;
}
