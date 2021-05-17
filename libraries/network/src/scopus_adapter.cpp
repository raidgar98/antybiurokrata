// Project Includes
#include <antybiurokrata/libraries/scopus_adapter/scopus_adapter.h>
#include <antybiurokrata/libraries/demangler/demangler.h>

// STL
#include <map>

namespace core
{
	namespace network
	{
		drogon::HttpRequestPtr scopus_adapter::prepare_request(const str& orcid, const size_t offset,
																				 const size_t count)
		{
			const str forcid = "orcid(" + orcid + ")";

			drogon::HttpRequestPtr req = drogon::HttpRequest::newHttpRequest();
			req->setMethod(drogon::Get);
			req->setPath("/content/search/scopus");
			req->setParameter("query", forcid);
			req->setParameter("apiKey", "a4a3a6dec671a73561980f44712ef25b");
			req->setParameter("view", "standard");
			req->setParameter("httpAccept", "application/json");
			if(count > 25ul)
				log.warn() << "max count is 25 for this endpoint expect errors!" << logger::endl;
			req->setParameter("count", std::to_string(count));
			req->setParameter("start", std::to_string(offset));

			return req;
		}

		scopus_adapter::result_t scopus_adapter::get_person(const str& orcid)
		{
			result_t result_list{new value_t{}};
			value_t& list = *result_list;

			size_t offset{0};
			size_t count{25};
			size_t total_results{0};

			using jvalue = Json::Value;
			const auto empty_array
				 = jvalue{Json::ValueType::arrayValue};	// alternative return if array is expected
			const auto null_value = jvalue{
				 Json::ValueType::
					  nullValue};	 // alternative result if anything other that array is expected
			auto cengine				= get_conversion_engine();
			const u16str wide_orcid = cengine.from_bytes(orcid);

			const auto safe_get
				 = [&null_value, &cengine](const str& field, const jvalue& json) -> u16str {
				const jvalue& element = json.get(field, null_value);
				if(element == null_value) return u16str();
				else
					return cengine.from_bytes(element.asCString());
			};

			do {
				if(total_results) offset += count;
				const connection_handler::raw_response_t response
					 = send_request(prepare_request(orcid, offset, count));
				dassert{response.first == drogon::ReqResult::Ok, "expected 200 response code"_u8};
				log.info() << "successfully got response from `https://api.elsevier.com`"
							  << logger::endl;

				std::shared_ptr<jvalue> json{nullptr};
				try
				{
					json = response.second->getJsonObject();
				}
				catch(const std::exception& e)
				{
					log.error() << "cought `std::exception` while gathering json. what(): "
									<< logger::endl
									<< e.what() << logger::endl;
					throw;
				}
				catch(...)
				{
					log.error() << "cought unknown exception while gathering json" << logger::endl;
					throw;
				}

				dassert(json.get() != nullptr, "empty result or invalid json"_u8);

				const jvalue& search_results = json->get("search-results", null_value);
				dassert(search_results != null_value,
						  "invalid input, no `search-results` field in json"_u8);

				const jvalue& jtr = search_results.get("opensearch:totalResults", null_value);
				dassert(jtr != null_value, "expected totalResults to be a numeric string"_u8);
				total_results = std::stoi(jtr.asCString());
				if(total_results == 0)
				{
					log.warn() << "for orcid: `" << orcid << "` got empty result set" << logger::endl;
					return result_list;
				}
				else
				{
					log.info() << "got: " << std::min(offset + count, total_results) << " / "
								  << total_results << logger::endl;
				}

				const jvalue& entry = search_results.get("entry", empty_array);
				dassert(entry.isArray(), "entry has to be array"_u8);
				for(const jvalue& obj: entry)
				{
					detail::json_repr_t x{};

					x.title = safe_get("dc:title", obj);
					if(x.title.empty()) continue;

					x.year = safe_get("prism:coverDate", obj);
					if(x.year.empty()) continue;

					const u16str doi = safe_get("prism:doi", obj);
					if(!doi.empty()) x.ids.emplace_back(std::make_pair(u"doi", doi));

					const u16str issn = safe_get("prism:issn", obj);
					if(!issn.empty()) x.ids.emplace_back(std::make_pair(u"pissn", issn));

					const u16str eid = safe_get("eid", obj);
					if(!eid.empty()) x.ids.emplace_back(std::make_pair(u"eid", eid));

					x.orcid = wide_orcid;
					x.print();
					list.emplace_back(std::move(x));
				}

			} while(offset + count < total_results);

			return result_list;
		}
	}	 // namespace network
}	 // namespace core