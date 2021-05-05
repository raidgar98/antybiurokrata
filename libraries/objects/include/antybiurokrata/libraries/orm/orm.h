#pragma once

// Project includes
#include <antybiurokrata/libraries/bgpolsl_adapter/bgpolsl_adapter.h>
#include <antybiurokrata/libraries/objects/objects.h>

namespace core
{
	namespace orm
	{
		using namespace core::objects;
		using namespace core::network::detail;
		using publication_storage_t = std::shared_ptr<publication_t>;

		struct persons_extractor_t : Log<persons_extractor_t>,
			public patterns::visits<bgpolsl_repr_t>
		{
			using Log<persons_extractor_t>::log;

			std::set<person_t> persons;
			publication_storage_t current_publication{nullptr};

			virtual bool visit(bgpolsl_repr_t* ptr) override;
		};

		struct publications_extractor_t : Log<publications_extractor_t>,
			public patterns::visits<bgpolsl_repr_t>
		{
			using Log<publications_extractor_t>::log;

			explicit publications_extractor_t(persons_extractor_t& vs) : person_visitor{vs} {}

			persons_extractor_t& person_visitor;
			std::vector<publication_storage_t> publications{};

			virtual bool visit(bgpolsl_repr_t* ptr) override;
		};
	}
}
