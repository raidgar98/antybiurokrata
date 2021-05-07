#include <QApplication>
// #include <antybiurokrata/types.hpp>
#include <antybiurokrata/libraries/bgpolsl_adapter/bgpolsl_adapter.h>
#include <antybiurokrata/libraries/orcid_adapter/orcid_adapter.h>
#include <antybiurokrata/libraries/scopus_adapter/scopus_adapter.h>
#include <antybiurokrata/libraries/orm/orm.h>
// #include <antybiurokrata/windows/mainwindow/mainwindow.h>

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
	// QApplication a(argc, argv);
	// MainWindow w;
	// w.show();
	// for(const auto& x : *ret) x.print();

	///////////////////////////////////////////////////////////////////////////////////

	// bgpolsl
	core::network::bgpolsl_adapter bgadapter{};
	core::orm::persons_extractor_t bgperson_visitor{};
	core::orm::publications_extractor_t bgvisitor{bgperson_visitor};
	auto res = bgadapter.get_person(argv[1], argv[2]);
	for(auto& x: *res) x.accept(&bgvisitor);

	// orcid
	core::orm::persons_extractor_t orcid_person_visitor{bgperson_visitor};
	for(auto& x: orcid_person_visitor.persons) x().publictions().clear();
	core::orm::publications_extractor_t ovisitor{orcid_person_visitor};
	core::network::orcid_adapter oadapter{};
	for(const auto& person: orcid_person_visitor.persons)
	{
		auto ret = oadapter.get_person(person().orcid()());
		for(auto& x: *ret) x.accept(&ovisitor);
	}

	// scopus
	core::orm::persons_extractor_t scopus_person_visitor{bgperson_visitor};
	for(auto& x: scopus_person_visitor.persons) x().publictions().clear();
	core::orm::publications_extractor_t svisitor{scopus_person_visitor};
	core::network::scopus_adapter sadapter{};
	for(const auto& person: scopus_person_visitor.persons)
	{
		auto ret = sadapter.get_person(person().orcid()());
		for(auto& x: *ret) x.accept(&svisitor);
	}

	for(const auto& p: bgperson_visitor.persons)
		global_logger.info() << "[ size: " << p().publictions().size() << " ] " << patterns::serial::pretty_print{p}
									<< logger::endl;

	global_logger << "^^^^^^^^^^^^ BGPOLSL ^^^^^^^^^^^^" << logger::endl;

	for(const auto& p: orcid_person_visitor.persons)
		global_logger.info() << "[ size: " << p().publictions().size() << " ] " << patterns::serial::pretty_print{p}
									<< logger::endl;

	global_logger << "^^^^^^^^^^^^^ ORCID ^^^^^^^^^^^^^" << logger::endl;

	for(const auto& p: scopus_person_visitor.persons)
		global_logger.info() << "[ size: " << p().publictions().size() << " ] " << patterns::serial::pretty_print{p}
									<< logger::endl;

	global_logger << "^^^^^^^^^^^^^ SCOPUS ^^^^^^^^^^^^" << logger::endl;
	return 0;
	// return a.exec();
}
