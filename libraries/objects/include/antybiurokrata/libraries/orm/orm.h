#pragma once

// Project includes
#include <antybiurokrata/libraries/bgpolsl_adapter/bgpolsl_adapter.h>
#include <antybiurokrata/libraries/scopus_adapter/scopus_adapter.h>
#include <antybiurokrata/libraries/orcid_adapter/orcid_adapter.h>
#include <antybiurokrata/libraries/objects/objects.h>

namespace core
{
	namespace orm
	{
		using shared_person_t = std::shared_ptr<objects::person_t>;
		struct less_person_comparator
		{
			bool operator()(const shared_person_t& p1, const shared_person_t& p2) const;
		};

		using namespace core::objects;
		using namespace core::network::detail;
		using publication_storage_t = std::shared_ptr<publication_t>;
		using persons_storage_t		 = std::set<shared_person_t, less_person_comparator>;


		struct persons_extractor_t :
			 Log<persons_extractor_t>,
			 public patterns::visits<bgpolsl_repr_t>,
			 public patterns::visits<json_repr_t>
		{
			using Log<persons_extractor_t>::log;

			persons_storage_t persons;
			publication_storage_t current_publication{nullptr};

			static void shallow_copy_persons(const persons_extractor_t& input,
														persons_extractor_t& output);

			virtual bool visit(bgpolsl_repr_t* ptr) override;
			virtual bool visit(json_repr_t* ptr) override;
		};

		struct publications_extractor_t :
			 Log<publications_extractor_t>,
			 public patterns::visits<bgpolsl_repr_t>,
			 public patterns::visits<json_repr_t>
		{
			using Log<publications_extractor_t>::log;

			explicit publications_extractor_t(persons_extractor_t& vs) : person_visitor{vs} {}

			persons_extractor_t& person_visitor;
			std::vector<publication_storage_t> publications{};

			virtual bool visit(bgpolsl_repr_t* ptr) override;
			virtual bool visit(json_repr_t* ptr) override;
		};
	}	 // namespace orm
}	 // namespace core
