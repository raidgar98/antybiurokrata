/**
 * @file orm.h
 * @author Krzysztof Mochocki (raidgar98@onet.pl)
 * @brief contains declarations of visitors that converts raw objects to higher layer objects
 * @version 0.1
 * @date 2021-05-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

// Project includes
#include <antybiurokrata/libraries/bgpolsl_adapter/bgpolsl_adapter.h>
#include <antybiurokrata/libraries/scopus_adapter/scopus_adapter.h>
#include <antybiurokrata/libraries/orcid_adapter/orcid_adapter.h>
#include <antybiurokrata/libraries/objects/objects.h>

namespace core
{
	/** @brief contains converters from adapters output to objects */
	namespace orm
	{
		using shared_person_t = std::shared_ptr<objects::person_t>;
		/** @brief checks is pointers not null, and then forwards to operator to their content */
		struct less_person_comparator
		{
			bool operator()(const shared_person_t& p1, const shared_person_t& p2) const;
		};

		using namespace core::objects;
		using namespace core::network::detail;
		using publication_storage_t = std::shared_ptr<publication_t>;
		using persons_storage_t		 = std::set<shared_person_t, less_person_comparator>;

		/**
		 * @brief constructs person_t objects, by visiting output from adapters
		 */
		struct persons_extractor_t :
			 Log<persons_extractor_t>,
			 public patterns::visits<bgpolsl_repr_t>,
			 public patterns::visits<json_repr_t>
		{
			using Log<persons_extractor_t>::log;

			persons_storage_t persons;
			publication_storage_t current_publication{nullptr};

			/**
			 * @brief handy if you want names, but without publications
			 * 
			 * @param input sorce 
			 * @param output destination
			 */
			static void shallow_copy_persons(const persons_extractor_t& input,
														persons_extractor_t& output);

			virtual bool visit(bgpolsl_repr_t* ptr) override;
			virtual bool visit(json_repr_t* ptr) override;
		};

		/**
		 * @brief constructs publication_t objects, by visiting output from adapters
		 */
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
