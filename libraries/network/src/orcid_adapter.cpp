// Project Includes
#include <antybiurokrata/libraries/orcid_adapter/orcid_adapter.h>
#include <antybiurokrata/libraries/demangler/demangler.h>
#include <antybiurokrata/libraries/objects/objects.h>

// STL
#include <map>

namespace core
{
	namespace network
	{
		drogon::HttpRequestPtr orcid_adapter::prepare_request(const str& orcid)
		{
			const std::map<std::string, std::string> headers{{std::pair<std::string, std::string>{"Accept", "application/json"}}};

			drogon::HttpRequestPtr req = drogon::HttpRequest::newHttpRequest();
			req->setMethod(drogon::Get);
			for (const auto &kv : headers) req->addHeader(kv.first, kv.second);
			req->setPath("/v3.0/" + orcid + "/works");

			return req;
		}

		orcid_adapter::result_t orcid_adapter::get_person(const str& orcid)
		{
			result_t result_list{new value_t{}};
			value_t& list = *result_list;

			const connection_handler::raw_response_t response = send_request(prepare_request(orcid));
			dassert{response.first == drogon::ReqResult::Ok, "expected 200 response code"};
			log.info() << "successfully got response from `https://pub.orcid.org`" << logger::endl;

			// log.info() << "got json: " << response.second->body() << logger::endl;

			return result_list;
		}
	}
}