// Project Includes
#include <antybiurokrata/libraries/bgpolsl_adapter/bgpolsl_adapter.h>
#include <antybiurokrata/libraries/html_scalpel/html_scalpel.h>
#include <antybiurokrata/libraries/demangler/demangler.h>

// STL
#include <map>
#include <ranges>

namespace core
{
	namespace network
	{
		namespace detail
		{
			bgpolsl_repr_t::bgpolsl_repr_t(const std::vector<u16str> & words)
			{
				const std::map<u16str, u16str*> keywords{{std::pair<u16str, u16str*>
					{u"IDT", &idt}, {u"Rok", &year}, {u"Autorzy", &authors},
					{u"Tytuł oryginału", &org_title},{u"Tytuł całości", &whole_title},
					{u"Dokument elektroniczny", &e_doc}, {u"Czasopismo", nullptr},
					{u"Szczegóły", nullptr}, {u"p-ISSN", &p_issn}, {u"DOI", &doi},
					{u"e-ISSN", &e_issn}, {u"Adres", nullptr}, {u"Afiliacja", &affiliation},
					{u"Punktacja", nullptr}, {u"Pobierz", nullptr}, {u"Dyscypliny", nullptr},
					{u"Uwaga", nullptr}}};
				std::map<u16str_v, bool> double_keywords;
				for(const auto& pair : keywords)
				{
					const size_t pos = pair.first.find(u' ');
					if(pos != u16str::npos) /* if found */ double_keywords[u16str_v{ pair.first.c_str(), pos}] = false;
				}

				bool complex_active = false;
				u16str *savepoint = nullptr;
				u16str word;
				for (const u16str &i_word : words)
				{
					if (i_word.size() <= 1) continue;

					if (complex_active) for(auto& pair : double_keywords) if(pair.second)
					{
						word = pair.first;
						pair.second = false;
						complex_active = false;
						break;
					}

					const auto it = double_keywords.find(i_word);
					if (it != double_keywords.end())
					{
						it->second = true;
						complex_active = true;
						continue;
					}

					const u16str* search_point{ word.size() > 0 ? &word : &i_word };
					auto found = keywords.find(*search_point);
					if (found == keywords.end()) // if not found
					{
						if (savepoint != nullptr)
						{
							*savepoint += *search_point;
							*savepoint += u' ';
						}
					}
					else /* if found */ savepoint = found->second;
					word.clear();
				}
			}
		}

		drogon::HttpRequestPtr core::network::bgpolsl_adapter::prepare_request(const core::str_v &querried_name)
		{
			const std::map<std::string, std::string> headers{{std::pair<std::string, std::string>
				{"User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:87.0) Gecko/20100101 Firefox/87.0"},
				{"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"},
				{"Accept-Language", "en-US,en;q=0.5"},
				{"Content-Type", "application/x-www-form-urlencoded"},
				{"Origin", "https://www.bg.polsl.pl"},
				{"DNT", "1"},
				{"Connection", "keep-alive"},
				{"Referer", "https://www.bg.polsl.pl/expertus/new/bib/expwww.html"},
				{"Upgrade-Insecure-Requests", "1"}}
			};
			drogon::HttpRequestPtr req = drogon::HttpRequest::newHttpRequest();
			req->setMethod(drogon::Post);
			req->setPath("/expertusbin/expertus4.cgi");
			for (const auto &kv : headers) req->addHeader(kv.first, kv.second);
			req->setContentTypeCode(drogon::ContentType::CT_APPLICATION_X_FORM);
			req->setPathEncode(true);
			str body{"KAT=%2Fvar%2Fwww%2Fbibgl%2Fexpertusdata%2Fnew%2Fpar%2F&FST=data.fst&F_00=02&V_00="};
			body += querried_name;
			body += "&F_01=04&V_01=&F_02=07&V_02=&cond=AND&FDT=data98.fdt&fldset=&sort=-1%2C100a%2C150a%2C200a%2C250a%2C303a%2C350a%2C400a%2C450a%2C700a%2C750a&X_0=1&R_0=1000&plainform=0&ESF=01&ESF=02&ESF=07&ESF=08&ESS=stat.htm&STPL=ANALYSIS&ESK=1&sumpos=%7Bsumpos%7D&year00=0&ZA=&F_07=00&V_07=&F_31=94&V_31=&F_28=86&V_28=&F_23=98&V_23=&F_18=22&F_08=17&B_01=033&C_01=3&D_01=&F_21=41&F_14=21&F_04=16&B_00=015&C_00=3&D_00=&F_10=41&F_11=19&V_11=&F_05=40&V_05=&F_12=54&V_12=&F_32=91&V_32=&F_29=49&V_29=&F_09=53&V_09=&F_20=78&V_20=&F_16=57&F_06=25&F_22=88&F_30=88&V_30=&F_24=79&F_25=14&F_33=36&V_33=&F_15=55&V_15=&F_19=74&V_19=&F_13=26&druk=0&cfsect=&mask=1&ekran=ISO&I_XX=a";

			return req;
		}

		bgpolsl_adapter::result_t bgpolsl_adapter::get_person(const str_v &name, const str_v &surname)
		{
			constexpr str_v match_expresion{R"(<span class="field_id"><br/><span class="label" name="label_id">IDT:)"};
			bgpolsl_adapter::result_t result{ new value_t{} };

			str full_name{surname};
			full_name += ' ';
			full_name += name;

			const connection_handler::raw_response_t response = send_request(
				prepare_request(
					core::demangler<>{full_name}
						.process<conv_t::URL>()
						.get()
				)
			);

			dassert{response.first == drogon::ReqResult::Ok, "expected 200 response code"};
			log.info() << "successfully got response from `bg.polsl.pl`" << logger::endl;

			const str_v view{response.second->getBody()};
			const std::ranges::split_view splitted{view, '\n'};
			for (const auto &line : splitted)
			{
				str tmp;
				tmp.reserve(std::ranges::distance(line));
				for (const auto c : line) tmp += c;		// very very bad, potential optimalization point
				if (tmp.find(match_expresion) != std::string::npos)
				{
					std::vector<u16str> words;
					html_scalpel(tmp, words);
					result->emplace_back( words );	// bgpolsl_repr_t{}
				}
			}

			return result;
		}
	}
}