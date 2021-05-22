// Project Includes
#include <antybiurokrata/libraries/global_adapters.hpp>
#include <antybiurokrata/libraries/demangler/demangler.h>

// STL
#include <map>

namespace core
{
	namespace network
	{

		namespace global_adapters
		{
			orcid_adapter orcid{};
		}

		drogon::HttpRequestPtr orcid_adapter::prepare_request(const str& orcid)
		{
			const std::map<std::string, std::string> headers{
				 {std::pair<std::string, std::string>{"Accept", "application/json"}}};

			drogon::HttpRequestPtr req = drogon::HttpRequest::newHttpRequest();
			req->setMethod(drogon::Get);
			for(const auto& kv: headers) req->addHeader(kv.first, kv.second);
			req->setPath("/v3.0/" + orcid + "/works");

			return req;
		}

		drogon::HttpRequestPtr orcid_adapter::prepare_request_for_person(const str& orcid)
		{
			drogon::HttpRequestPtr result = prepare_request(orcid);
			result->setPath("/v3.0/" + orcid + "/person");
			return result;
		}

		void orcid_adapter::get_name_and_surname(const str& orcid, str& out_name, str& out_surname)
		{
			auto try_split = [&](const str_v& view) -> bool {
				core::string_utils::split_words<str_v> splitter{view, ' '};
				auto first	= splitter.begin();
				auto second = splitter.begin();
				second++;
				if(first != splitter.end() && !(*first).empty() && second != splitter.end()
					&& !(*second).empty())
				{
					out_name		= *first;
					out_surname = *second;
					return true;
				}
				else
					return false;
			};

			const connection_handler::raw_response_t response
				 = send_request(prepare_request_for_person(orcid));
			dassert{response.first == drogon::ReqResult::Ok, "expected 200 response code"_u8};
			log.info() << "successfully got response from `https://pub.orcid.org`" << logger::endl;

			using jvalue			  = Json::Value;
			const auto null_value  = jvalue{Json::ValueType::nullValue};
			const auto empty_array = jvalue{Json::ValueType::arrayValue};
			std::shared_ptr<jvalue> json{nullptr};

			try
			{
				json = response.second->getJsonObject();
			}
			catch(const std::exception& e)
			{
				log.error() << "cought `std::exception` while gathering json. what(): " << logger::endl
								<< e.what() << logger::endl;
			}
			catch(...)
			{
				log.error() << "cought unknown exception while gathering json" << logger::endl;
			}

			dassert(json.get() != nullptr, "empty result or invalid json"_u8);

			const jvalue& name = json->get("name", null_value);
			if(name != null_value) [[likely]]
			{
				const jvalue& given_names = name.get("given-names", null_value);
				const jvalue& family_name = name.get("family-name", null_value);
				str to_split{};

				if(family_name == null_value && given_names != null_value)
				{
					const jvalue& value = given_names.get("value", null_value);
					if(value != null_value && value.isString()) to_split = value.asCString();
				}
				else if(given_names == null_value && family_name != null_value) [[unlikely]]
				{
					const jvalue& value = family_name.get("value", null_value);
					if(value != null_value && value.isString()) to_split = value.asCString();
				}
				else if(given_names != null_value && family_name != null_value) [[likely]]
				{
					const jvalue& name	 = given_names.get("value", null_value);
					const jvalue& surname = family_name.get("value", null_value);
					if(name != null_value && surname != null_value) [[likely]]
					{
						dassert(name.isString(), "name is not a string"_u8);
						dassert(surname.isString(), "surname is not a string"_u8);
						out_name		= name.asCString();
						out_surname = surname.asCString();
						return;
					}
				}
				if(!to_split.empty() && try_split(to_split)) return;
			}

			const jvalue& addresses = json->get("addresses", null_value);
			if(addresses != null_value)
			{
				const jvalue& address_arr = addresses.get("address", empty_array);
				if(address_arr.isArray())
				{
					for(const jvalue& item: address_arr)
					{
						const jvalue& source = item.get("source", null_value);
						if(source != null_value)
						{
							const jvalue& source_name = source.get("source-name", null_value);
							if(source_name != null_value)
							{
								const jvalue& value = source_name.get("value", null_value);
								if(value != null_value && value.isString())
								{
									const str x{value.asCString()};
									if(try_split(x)) return;
								}
							}
						}
					}
				}
			}

			constexpr str_v not_found{"cannot extract name from given orcid"};
			const jvalue& external_identifiers = json->get("external-identifiers", null_value);
			dassert(external_identifiers != null_value, not_found);
			const jvalue& external_identifier
				 = external_identifiers.get("external-identifier", empty_array);
			dassert(external_identifier.isArray() && external_identifier.size() > 0, not_found);
			for(const jvalue& item: external_identifier)
			{
				const jvalue& source = item.get("source", null_value);
				if(source != null_value)
				{
					const jvalue& assertion_name = source.get("assertion-origin-name", null_value);
					if(assertion_name != null_value)
					{
						const jvalue& value = assertion_name.get("value", null_value);
						if(value != null_value && value.isString())
						{
							const str x{value.asCString()};
							if(try_split(x)) return;
						}
					}
				}
			}
			dassert(false, not_found);
		}

		orcid_adapter::result_t orcid_adapter::get_person(const str& orcid)
		{
			result_t result_list{new value_t{}};
			value_t& list = *result_list;

			const connection_handler::raw_response_t response = send_request(prepare_request(orcid));
			dassert{response.first == drogon::ReqResult::Ok, "expected 200 response code"_u8};
			log.info() << "successfully got response from `https://pub.orcid.org`" << logger::endl;

			using jvalue = Json::Value;
			const auto empty_array
				 = jvalue{Json::ValueType::arrayValue};	// alternative return if array is expected
			const auto null_value = jvalue{
				 Json::ValueType::
					  nullValue};	 // alternative result if anything other that array is expected

			std::shared_ptr<jvalue> json{nullptr};
			try
			{
				json = response.second->getJsonObject();
			}
			catch(const std::exception& e)
			{
				log.error() << "cought `std::exception` while gathering json. what(): " << logger::endl
								<< e.what() << logger::endl;
			}
			catch(...)
			{
				log.error() << "cought unknown exception while gathering json" << logger::endl;
			}

			dassert(json.get() != nullptr, "empty result or invalid json"_u8);

			const jvalue& array = json->get("group", empty_array);
			dassert(array.isArray(), "it's not array"_u8);
			log << "it's array, with size: " << array.size() << " hooray!" << logger::endl;
			if(array.size() == 0) log.warn() << "array is empty for orcid: " << orcid << logger::endl;
			auto cengine				= get_conversion_engine();
			const u16str wide_orcid = cengine.from_bytes(orcid);

			using namespace network::detail;
			for(const jvalue& x: array)
			{
				detail::json_repr_t obj{};
				obj.orcid = wide_orcid;

				const jvalue& work_summary = x.get("work-summary", empty_array);
				if(work_summary.isArray() && work_summary.size() > 0)
				{
					const jvalue& item_0 = work_summary[0];

					{	 // year
						const jvalue& pub_date = item_0.get("publication-date", null_value);
						if(pub_date == null_value) continue;

						const jvalue& year = pub_date.get("year", null_value);
						if(year == null_value) continue;
						else
							obj.year = cengine.from_bytes(year["value"].asCString());
						log << "added year" << logger::endl;
					}

					{	 // title
						const jvalue& pretitle = item_0.get("title", null_value);
						if(pretitle == null_value) continue;
						constexpr u16char_t double_tittle_separator{u','};
						bool double_title = false;

						{	 // orginal
							const jvalue& title = pretitle.get("title", null_value);
							if(title == null_value) continue;
							else
								obj.title = cengine.from_bytes(title["value"].asCString());
							if(obj.title.find(double_tittle_separator)) double_title = true;
							log << "added tittle" << logger::endl;
						}

						if(!double_title) /* i hope */ [[likely]]
						{
							const jvalue& title = pretitle.get("translated-title", null_value);
							if(title != null_value)
								obj.translated_title = cengine.from_bytes(title["value"].asCString());
							log << "added translated tittle" << logger::endl;
						}
						else
						{
							const string_utils::split_words<u16str_v> splitter{obj.title,
																								double_tittle_separator};
							auto it = splitter.begin();
							u16str first_title{*it};
							it++;
							obj.translated_title = *it;
							obj.title				= std::move(first_title);
						}
					}
				}
				else
					continue;
				log << "properly added year and tittle" << logger::endl;

				// ids
				const jvalue& external_ids = x.get("external-ids", null_value);
				if(external_ids == null_value) continue;
				const jvalue& external_id = external_ids.get("external-id", empty_array);
				if(external_id.isArray() && external_id.size() > 0)
				{
					log << "searching for ids in array of size: " << external_id.size() << logger::endl;
					for(const jvalue& item: external_id)
					{
						std::pair<u16str, u16str> to_emplace{};

						const jvalue& eid_type = item.get("external-id-type", null_value);
						if(eid_type == null_value) continue;
						else
							to_emplace.first = cengine.from_bytes(eid_type.asCString());

						const jvalue& eid_normalized = item.get("external-id-normalized", null_value);
						if(eid_normalized == null_value)
						{
							const jvalue& eid_wild = item.get("external-id-value", null_value);
							if(eid_wild == null_value) continue;

							to_emplace.second = cengine.from_bytes(eid_wild.asCString());
						}
						else
							to_emplace.second = cengine.from_bytes(eid_normalized["value"].asCString());

						log << "added id: ( " << to_emplace.first << " ; " << to_emplace.second << " )"
							 << logger::endl;
						obj.ids.emplace_back(std::move(to_emplace));
					}
				}
				log << "properly added ids" << logger::endl;

				list.emplace_back(std::move(obj));
				log << "properly added publication" << logger::endl;
			}

			return result_list;
		}
	}	 // namespace network
}	 // namespace core