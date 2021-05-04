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

		struct persons_extractor_t : Log<persons_extractor_t>,
			public patterns::visits<bgpolsl_repr_t>
		{
			using Log<persons_extractor_t>::log;
			std::set<person_t> persons;

			virtual bool visit(bgpolsl_repr_t* ptr) override;
		};

		struct publications_extractor_t : Log<publications_extractor_t>,
			public patterns::visits<bgpolsl_repr_t>
		{
			using Log<publications_extractor_t>::log;
			persons_extractor_t& person_visitor;
			std::set<publication_t> persons;

			virtual bool visit(bgpolsl_repr_t* ptr) override;
		};
	}
}
