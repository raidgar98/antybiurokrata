#include <locale>
#include <iostream>
#include <string_view>
#include <map>
#include <string>
#include <list>
#include <future>
#include <cctype>
#include <drogon/drogon.h>
#include <fstream>
#include <ranges>
#include <set>
// #include "static_serial.hpp"

#include <boost/thread/concurrent_queues/sync_queue.hpp>

// const auto _ = R"(

// curl
// 'https://www.bg.polsl.pl/expertusbin/expertus4.cgi'
// --compressed

// --data-raw 'KAT=%2Fvar%2Fwww%2Fbibgl%2Fexpertusdata%2Fnew%2Fpar%2F&FST=data.fst&F_00=02&V_00=SMAG%C3%93R+ADRIAN&F_01=04&V_01=&F_02=07&V_02=&cond=AND&FDT=data98.fdt&fldset=&sort=-1%2C100a%2C150a%2C200a%2C250a%2C303a%2C350a%2C400a%2C450a%2C700a%2C750a&X_0=1&R_0=1000&plainform=0&ESF=01&ESF=02&ESF=07&ESF=08&ESS=stat.htm&STPL=ANALYSIS&ESK=1&sumpos=%7Bsumpos%7D&year00=0&ZA=&F_07=00&V_07=&F_31=94&V_31=&F_28=86&V_28=&F_23=98&V_23=&F_18=22&F_08=17&B_01=033&C_01=3&D_01=&F_21=41&F_14=21&F_04=16&B_00=015&C_00=3&D_00=&F_10=41&F_11=19&V_11=&F_05=40&V_05=&F_12=54&V_12=&F_32=91&V_32=&F_29=49&V_29=&F_09=53&V_09=&F_20=78&V_20=&F_16=57&F_06=25&F_22=88&F_30=88&V_30=&F_24=79&F_25=14&F_33=36&V_33=&F_15=55&V_15=&F_19=74&V_19=&F_13=26&druk=0&cfsect=&mask=1&ekran=ISO&I_XX=a'

// )";

const std::map<std::string, std::string> headers{{std::pair<std::string, std::string>{"User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:87.0) Gecko/20100101 Firefox/87.0"},
												  {"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"},
												  {"Accept-Language", "en-US,en;q=0.5"},
												  {"Content-Type", "application/x-www-form-urlencoded"},
												  {"Origin", "https://www.bg.polsl.pl"},
												  {"DNT", "1"},
												  {"Connection", "keep-alive"},
												  {"Referer", "https://www.bg.polsl.pl/expertus/new/bib/expwww.html"},
												  {"Upgrade-Insecure-Requests", "1"}}};

template <class Facet>
struct deletable_facet : Facet
{
	template <class... Args>
	deletable_facet(Args &&...args) : Facet(std::forward<Args>(args)...) {}
	~deletable_facet() {}
};

struct dorobek_repr_t
{
	using str = std::string;

	str idt;
	str year;
	str authors;
	str org_title;
	str whole_title;
	str e_doc;
	str p_issn;
	str doi;
	str e_issn;
	str affiliation;

	explicit dorobek_repr_t(const std::string &data)
	{
		const std::map<str, str *> keywords{{std::pair<std::string, str *>{"IDT", &idt},
			{"Rok", &year}, {"Autorzy", &authors}, {"Tytuł oryginału", &org_title},{"Tytuł całości", &whole_title},
			{"Dokument elektroniczny", &e_doc}, {"Czasopismo", nullptr}, {"Szczegóły", nullptr}, {"p-ISSN", &p_issn},
			{"DOI", &doi}, {"e-ISSN", &e_issn}, {"Adres", nullptr}, {"Afiliacja", &affiliation}, 
			{"Punktacja", nullptr}, {"Pobierz", nullptr}, {"Dyscypliny", nullptr}, {"Uwaga", nullptr}}};

		const std::string_view view{data};
		const std::ranges::split_view splitted{view, ' '};

		bool is_title = false;
		bool is_doc = false;
		str* savepoint = nullptr;
		for(const auto& part : splitted)
		{
			const long distance{ std::ranges::distance(part) };
			if(distance <= 1) continue;
			std::string word;

			if(is_title)
			{
				word = "Tytuł ";
				is_title = false;
			}else if(is_doc)
			{
				word = "Dokument ";
				is_doc = false;
			}


			word.reserve(distance);
			for (const auto c : part) word += c;

			if(word == "Tytuł") { is_title = true; continue; }
			else if(word == "Dokument") { is_doc = true; continue; }
			auto found = keywords.find(word);
			if(found == keywords.end()){ if(savepoint != nullptr) {*savepoint += word; *savepoint += " ";} }
			else savepoint = found->second;
		}
	}

	void print(std::ostream &os)
	{
		os << "IDT: " << idt << std::endl;
		os << "YEAR: " << year << std::endl;
		os << "AUTHORS: " << authors << std::endl;
		os << "ORG_TITLE: " << org_title << std::endl;
		os << "WHOLE_TITLE: " << whole_title << std::endl;
		os << "E_DOC: " << e_doc << std::endl;
		os << "P_ISSN: " << p_issn << std::endl;
		os << "DOI: " << doi << std::endl;
		os << "E_ISSN: " << e_issn << std::endl;
		os << "AFFILIATION: " << affiliation << std::endl;
		os << "#########################################################\n";
	}
};

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
	queue.wait_push(value_t(sub_value_t{req, [&](drogon::ReqResult, const drogon::HttpResponsePtr &res) { responses.push(std::string{res->getBody()}); }}));
	queue.wait_push(value_t{});
	constexpr std::string_view match_expresion{R"(<span class="field_id"><br/><span class="label" name="label_id">IDT:)"};

	std::list<std::string> lines;

	{
		std::string result;
		responses.wait_pull(result);
		const std::string_view view{result};
		const std::ranges::split_view splitted{view, '\n'};
		for (const auto &line : splitted)
		{
			std::string tmp;
			tmp.reserve(std::ranges::distance(line));
			for (const auto c : line)
				tmp += c;
			if (tmp.find(match_expresion) != std::string::npos)
				lines.emplace_back(std::move(tmp));
		}
	}

	const auto is_correct = [](const wchar_t c) -> bool {
		const std::locale plPL{"pl_PL.UTF-8"};
		constexpr std::wstring_view accepted{L"-_ \t&#+/."};
		return std::isalpha(c, plPL) ||
			   std::isdigit(c, plPL) ||
			   accepted.find(c) != std::wstring_view::npos;
	};

	std::wstring_convert<
		deletable_facet<
			std::codecvt<
				char16_t,
				char,
				std::mbstate_t>>,
		char16_t>
		converter;

	for (auto &v : lines)
	{
		std::string output;
		output.reserve(v.size());

		bool in_tag = false;
		bool in_quote = false;
		bool ignore_next = false;
		bool is_double_spaced = false;

		for (const auto c : converter.from_bytes(v))
		{
			if (ignore_next)
				ignore_next = false;
			else if (c == L'\\')
				ignore_next = true;
			else if (in_tag && (c == L'"' || c == L'\''))
				in_quote = !in_quote;
			else if (!in_tag && c == L'<')
				in_tag = true;
			else if (in_tag && c == L'>')
			{
				in_tag = false;
				if (!is_double_spaced)
				{
					output += ' ';
					is_double_spaced = true;
				}
			}
			else if (!in_tag && is_correct(c))
			{
				if (is_double_spaced && c == L'.')
					continue;
				if (c == L' ' || c == L'\t')
				{
					if (is_double_spaced)
						continue;
					else
						is_double_spaced = true;
				}
				else
					is_double_spaced = false;

				output += converter.to_bytes(c);
			}
		}

		output.shrink_to_fit();
		v = output;
	}

	for (const auto &v : lines)
		dorobek_repr_t{v}.print(std::cout);

	return 0;
}