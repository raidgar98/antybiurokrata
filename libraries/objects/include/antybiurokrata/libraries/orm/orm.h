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
		using namespace core::objects;
		using namespace core::network::detail;
		using publication_storage_t = std::shared_ptr<publication_t>;

		struct persons_extractor_t :
			 Log<persons_extractor_t>,
			 public patterns::visits<bgpolsl_repr_t>,
			 public patterns::visits<json_repr_t>
		{
			using Log<persons_extractor_t>::log;
			using wrap_person_t = std::shared_ptr<person_t>;
			struct less_person_comparator
			{
				bool operator()(const wrap_person_t& p1, const wrap_person_t& p2) const
				{
					dassert{p1 && p2, "both pointers cannot be nullptr"_u8};
					return *p1 < *p2;
				}
			};

			std::set<wrap_person_t, less_person_comparator> persons;
			publication_storage_t current_publication{nullptr};

			static void shallow_copy_persons(const persons_extractor_t& input,
														persons_extractor_t& output)
			{
				for(const auto& person: input.persons)
				{
					wrap_person_t np{new person_t{}};
					(*np)().name	 = (*person)().name;
					(*np)().surname = (*person)().surname;
					(*np)().orcid	 = (*person)().orcid;
					auto pair = output.persons.emplace(np);
					if(pair.second) (*pair.first->get())().publictions()()->clear();
				}
			}

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
