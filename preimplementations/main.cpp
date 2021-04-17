#include <locale>
#include <iostream>
#include <map>
#include <string>
#include <future>
#include <drogon/drogon.h>
#include <fstream>
// #include "static_serial.hpp"

#include <boost/thread/concurrent_queues/sync_queue.hpp>

// const auto _ = R"(

// curl
// 'https://www.bg.polsl.pl/expertusbin/expertus4.cgi'
// --compressed

// --data-raw 'KAT=%2Fvar%2Fwww%2Fbibgl%2Fexpertusdata%2Fnew%2Fpar%2F&FST=data.fst&F_00=02&V_00=SMAG%C3%93R+ADRIAN&F_01=04&V_01=&F_02=07&V_02=&cond=AND&FDT=data98.fdt&fldset=&sort=-1%2C100a%2C150a%2C200a%2C250a%2C303a%2C350a%2C400a%2C450a%2C700a%2C750a&X_0=1&R_0=1000&plainform=0&ESF=01&ESF=02&ESF=07&ESF=08&ESS=stat.htm&STPL=ANALYSIS&ESK=1&sumpos=%7Bsumpos%7D&year00=0&ZA=&F_07=00&V_07=&F_31=94&V_31=&F_28=86&V_28=&F_23=98&V_23=&F_18=22&F_08=17&B_01=033&C_01=3&D_01=&F_21=41&F_14=21&F_04=16&B_00=015&C_00=3&D_00=&F_10=41&F_11=19&V_11=&F_05=40&V_05=&F_12=54&V_12=&F_32=91&V_32=&F_29=49&V_29=&F_09=53&V_09=&F_20=78&V_20=&F_16=57&F_06=25&F_22=88&F_30=88&V_30=&F_24=79&F_25=14&F_33=36&V_33=&F_15=55&V_15=&F_19=74&V_19=&F_13=26&druk=0&cfsect=&mask=1&ekran=ISO&I_XX=a'

// )";

const std::map<std::string, std::string> headers{{std::pair<std::string, std::string>
	{"User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:87.0) Gecko/20100101 Firefox/87.0"},
	{"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"},
	{"Accept-Language", "en-US,en;q=0.5"},
	{"Content-Type", "application/x-www-form-urlencoded"},
	{"Origin", "https://www.bg.polsl.pl"},
	{"DNT", "1"},
	{"Connection", "keep-alive"},
	{"Referer", "https://www.bg.polsl.pl/expertus/new/bib/expwww.html"},
	{"Upgrade-Insecure-Requests", "1"}
}};

int main()
{
	std::locale::global(std::locale("pl_PL.UTF-8"));

	using response_fun_t = std::function<void(drogon::ReqResult, const drogon::HttpResponsePtr &)>;
	using sub_value_t = std::pair<drogon::HttpRequestPtr, response_fun_t>;
	using value_t = std::optional<sub_value_t>;

	boost::concurrent::sync_queue<value_t> queue{};
	boost::concurrent::sync_queue<std::string> responses{};

	auto th = std::jthread([&queue]() {
		auto client = drogon::HttpClient::newHttpClient("https://www.bg.polsl.pl");

		auto ttt = std::jthread([&client]() {
			client->getLoop()->moveToCurrentThread();
			client->getLoop()->loop();
		});

		value_t ret;
		queue.wait_pull(ret);
		while (ret.has_value())
		{
			const auto result = client->sendRequest(ret->first);
			ret->second(result.first, result.second);

			ret.reset();
			queue.wait_pull(ret);
		}
		client->getLoop()->quit();
		std::cout << "exiting\n";
	});

	auto req = drogon::HttpRequest::newHttpRequest();
	req->setMethod(drogon::Post);
	req->setPath("/expertusbin/expertus4.cgi");
	for (const auto &kv : headers)
		req->addHeader(kv.first, kv.second);
	req->setContentTypeCode(drogon::ContentType::CT_APPLICATION_X_FORM);
	req->setPathEncode(true);
	//  req->setBody("KAT=%2Fvar%2Fwww%2Fbibgl%2Fexpertusdata%2Fnew%2Fpar%2F&FST=data.fst&F_00=02&V_00=BARGLIK+JERZY&F_01=04&V_01=&F_02=07&V_02=&cond=AND&FDT=data98.fdt&fldset=&sort=-1%2C100a%2C150a%2C200a%2C250a%2C303a%2C350a%2C400a%2C450a%2C700a%2C750a&X_0=1&R_0=1000&plainform=0&ESF=01&ESF=02&ESF=07&ESF=08&ESS=stat.htm&STPL=ANALYSIS&ESK=1&sumpos=%7Bsumpos%7D&year00=0&ZA=&F_07=00&V_07=&F_31=94&V_31=&F_28=86&V_28=&F_23=98&V_23=&F_18=22&F_08=17&B_01=033&C_01=3&D_01=&F_21=41&F_14=21&F_04=16&B_00=015&C_00=3&D_00=&F_10=41&F_11=19&V_11=&F_05=40&V_05=&F_12=54&V_12=&F_32=91&V_32=&F_29=49&V_29=&F_09=53&V_09=&F_20=78&V_20=&F_16=57&F_06=25&F_22=88&F_30=88&V_30=&F_24=79&F_25=14&F_33=36&V_33=&F_15=55&V_15=&F_19=74&V_19=&F_13=26&druk=0&cfsect=&mask=1&ekran=ISO&I_XX=a");
	req->setBody("KAT=%2Fvar%2Fwww%2Fbibgl%2Fexpertusdata%2Fnew%2Fpar%2F&FST=data.fst&F_00=02&V_00=SMAG%C3%93R+ADRIAN&F_01=04&V_01=&F_02=07&V_02=&cond=AND&FDT=data98.fdt&fldset=&sort=-1%2C100a%2C150a%2C200a%2C250a%2C303a%2C350a%2C400a%2C450a%2C700a%2C750a&X_0=1&R_0=1000&plainform=0&ESF=01&ESF=02&ESF=07&ESF=08&ESS=stat.htm&STPL=ANALYSIS&ESK=1&sumpos=%7Bsumpos%7D&year00=0&ZA=&F_07=00&V_07=&F_31=94&V_31=&F_28=86&V_28=&F_23=98&V_23=&F_18=22&F_08=17&B_01=033&C_01=3&D_01=&F_21=41&F_14=21&F_04=16&B_00=015&C_00=3&D_00=&F_10=41&F_11=19&V_11=&F_05=40&V_05=&F_12=54&V_12=&F_32=91&V_32=&F_29=49&V_29=&F_09=53&V_09=&F_20=78&V_20=&F_16=57&F_06=25&F_22=88&F_30=88&V_30=&F_24=79&F_25=14&F_33=36&V_33=&F_15=55&V_15=&F_19=74&V_19=&F_13=26&druk=0&cfsect=&mask=1&ekran=ISO&I_XX=a");

	std::cout << "pushing...\n";
	queue.wait_push(value_t(sub_value_t{req, [&](drogon::ReqResult, const drogon::HttpResponsePtr &res) { responses.push(std::string{ res->getBody() }); }}));
	queue.wait_push(value_t{});

	std::string result;
	responses.wait_pull(result);

	std::cout << result;

	return 0;
}

/*
int main_2()
{
    Example e{};
    std::cout << pretty_print{e} << std::endl;

    std::ofstream ofile{"serial.bin", std::ios::binary};
    ofile << e;
    // e.serialize(ofile);
    ofile.close();

    std::cout << std::endl;
    std::cout << e << std::endl;
    std::cout << std::endl;

    e.val.a.val = 0;
    e.val.b.val = 0;
    e.val.c.val = 0;

    std::ifstream ifile{"serial.bin", std::ios::binary};
    ifile >> e;
    // e.deserialize(ifile);
    ifile.close();

    std::cout << pretty_print{e} << std::endl;

    std::cout << "\n###################\n\n";

    MyStruct row{ 20.0060, "marcin", 2 };
    std::cout << row << std::endl;
    std::cout << pretty_print{row} << std::endl;

    std::ofstream ofile2{"serial.bin", std::ios::binary};
    ofile2 << row;
    ofile2.close();

    std::cout << row << std::endl;
    std::cout << pretty_print{row} << std::endl;

    std::ifstream ifile2{"serial.bin", std::ios::binary};
    ifile2 >> row;
    ifile2.close();

    std::cout << row << std::endl;
    std::cout << pretty_print{row} << std::endl;

    return 0;
}*/