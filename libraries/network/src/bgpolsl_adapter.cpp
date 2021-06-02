// Project Includes
#include <antybiurokrata/libraries/global_adapters.hpp>
#include <antybiurokrata/libraries/html_scalpel/html_scalpel.h>
#include <antybiurokrata/libraries/demangler/demangler.h>

// STL
#include <map>
#include <ranges>

namespace core
{
	namespace network
	{
		namespace global_adapters
		{
			bgpolsl_adapter polsl{};
		}

		namespace detail
		{
			void bgpolsl_repr_t::print() const
			{
				log.info() << "idt: " << idt << logger::endl;
				log.info() << "year: " << year << logger::endl;
				log.info() << "authors: " << authors << logger::endl;
				log.info() << "org_title: " << org_title << logger::endl;
				log.info() << "whole_title: " << whole_title << logger::endl;
				log.info() << "p_issn: " << p_issn << logger::endl;
				log.info() << "doi: " << doi << logger::endl;
				log.info() << "e_issn: " << e_issn << logger::endl;
				log.info() << "affiliation: " << affiliation << logger::endl;
			}

			bgpolsl_repr_t::bgpolsl_repr_t(const std::vector<u16str>& words)
			{
				const std::map<u16str, u16str*> keywords{{std::pair<u16str, u16str*>{u"IDT", &idt},
				{u"Rok", &year}, {u"Autorzy", &authors}, {u"Tytuł oryginału", &org_title},
				{u"Tytuł całości", &whole_title}, {u"Czasopismo", nullptr}, {u"Szczegóły", nullptr},
				{u"p-ISSN", &p_issn}, {u"DOI", &doi}, {u"Impact Factor", nullptr},
				{u"e-ISSN", &e_issn}, {u"Adres", nullptr}, {u"Afiliacja", &affiliation},
				{u"Punktacja", nullptr}, {u"Pobierz", nullptr}, {u"Dyscypliny", nullptr},
				{u"Uwaga", nullptr}}};

				u16str* savepoint = nullptr;
				for(const u16str& word: words)
				{
					u16str save_range;
					for(const auto& kv: keywords)
					{
						const size_t pos = word.find(kv.first);
						if(pos != u16str::npos) /* if found */
						{
							savepoint  = kv.second;
							save_range = u16str_v{word.c_str() + pos + 1ul + kv.first.size()};
							break;
						}
						else save_range = word;
					}

					if(savepoint)
					{
						if(savepoint->size() > 0) *savepoint += u' ';
						core::demangler<u16str, u16str_v>::mangle<conv_t::HTML>(save_range);
						*savepoint += save_range;
					}
				}
			}
		}	 // namespace detail

		drogon::HttpRequestPtr core::network::bgpolsl_adapter::prepare_request(
			 const core::str_v& querried_name)
		{
			const std::map<std::string, std::string> headers{
				 {std::pair<std::string, std::string>{
						"User-Agent",
						"Mozilla/5.0 (X11; Linux x86_64; rv:88.0) Gecko/20100101 Firefox/88.0"},
				  {"Accept",
					"text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"},
				  {"Accept-Language", "en-US,en;q=0.5"},
				  {"Content-Type", "application/x-www-form-urlencoded"},
				  {"Origin", "https://www.bg.polsl.pl"},
				  {"DNT", "1"},
				  {"Cookie", "NULL"},
				  {"Connection", "keep-alive"},
				  {"Referer", "https://www.bg.polsl.pl/expertus/new/bib/expwww.html"},
				  {"Upgrade-Insecure-Requests", "1"}}};

			drogon::HttpRequestPtr req = drogon::HttpRequest::newHttpRequest();
			req->setMethod(drogon::Post);
			req->setPath("/expertusbin/expertus4.cgi");
			for(const auto& kv: headers) req->addHeader(kv.first, kv.second);
			req->setContentTypeCode(drogon::ContentType::CT_APPLICATION_X_FORM);
			req->setPathEncode(true);

			// generating body for request
			str body{
				 "KAT=%2Fvar%2Fwww%2Fbibgl%2Fexpertusdata%2Fnew%2Fpar%2F&FST=data.fst&F_00=02&V_00="};
			body += querried_name;
			body
				 += "&F_01=04&V_01=&F_02=07&V_02=&cond=AND&FDT=data98.fdt&fldset=&sort=-1%2C100a%2C150a%2C200a%2C250a%2C303a%2C350a%2C400a%2C450a%2C700a%2C750a&X_0=1&R_0=5000&plainform=0&ESF=01&ESF=02&ESF=07&ESF=08&ESS=stat.htm&STPL=ANALYSIS&ESK=1&sumpos=%7Bsumpos%7D&year00=0&ZA=&F_07=00&V_07=&F_31=94&V_31=&F_28=86&V_28=&F_23=98&V_23=&F_18=22&F_08=17&B_01=033&C_01=3&D_01=&F_21=41&F_14=21&F_04=16&B_00=015&C_00=3&D_00=&F_10=41&F_11=19&V_11=&F_05=40&V_05=&F_12=54&V_12=&F_32=91&V_32=&F_29=49&V_29=&F_09=53&V_09=&F_20=78&V_20=&F_16=57&F_06=25&F_22=88&F_30=88&V_30=&F_24=79&F_25=14&F_33=36&V_33=&F_15=55&V_15=&F_19=74&V_19=&F_13=26&druk=0&cfsect=&mask=1&ekran=ISO&I_XX=a";

			req->setBody(body);

			return req;
		}

		bgpolsl_adapter::result_t bgpolsl_adapter::get_person(const str_v& name, const str_v& surname)
		{
			constexpr str_v match_expresion{
				 R"(<span class="field_id"><br/><span class="label" name="label_id">IDT:)"};
			bgpolsl_adapter::result_t result{new value_t{}};

			str full_name{surname};
			full_name += ' ';
			full_name += name;

			const connection_handler::raw_response_t response = send_request(
				 prepare_request(core::demangler<>{full_name}.process<conv_t::URL>().get()));


			dassert{response.first == drogon::ReqResult::Ok, "expected 200 response code"_u8};
			log.info() << "successfully got response from `https://www.bg.polsl.pl`" << logger::endl;

			const str_v view{response.second->getBody()};
			for(str_v line: string_utils::split_words<str_v>{view, '\n'})
			{
				if(line.find(match_expresion) != std::string::npos)
				{
					str tmp{line};
					std::vector<u16str> words;
					html_scalpel(tmp, words);
					result->emplace_back(words);	 // bgpolsl_repr_t{}
				}
			}

			return result;
		}
	}	 // namespace network
}	 // namespace core