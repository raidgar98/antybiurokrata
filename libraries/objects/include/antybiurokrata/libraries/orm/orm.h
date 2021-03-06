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
#include <antybiurokrata/libraries/global_adapters.hpp>
#include <antybiurokrata/libraries/objects/objects.h>

namespace core
{
	/** @brief contains converters from adapters output to objects */
	namespace orm
	{
		using namespace core::objects;
		using namespace core::network::detail;

		using shared_person_t		= objects::shared_person_t;
		using shared_publication_t = objects::shared_publication_t;
		using persons_storage_t		= std::shared_ptr<std::set<shared_person_t>>;

		/**
		 * @brief constructs person_t objects, by visiting output from adapters
		 */
		struct persons_extractor_t :
			 Log<persons_extractor_t>,
			 public patterns::visits<bgpolsl_repr_t>,
			 public patterns::visits<json_repr_t>
		{
			using Log<persons_extractor_t>::log;

			persons_storage_t persons{new persons_storage_t::element_type{}};
			shared_publication_t current_publication{nullptr};

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

			explicit publications_extractor_t(persons_extractor_t& vs) : person_visitor{vs}
			{
				publications.reserve(250ul);
			}

			persons_extractor_t& person_visitor;
			std::vector<shared_publication_t> publications{};

			virtual bool visit(bgpolsl_repr_t* ptr) override;
			virtual bool visit(json_repr_t* ptr) override;
		};
	}	 // namespace orm

	namespace network
	{
		namespace global_adapters
		{
			/**
			 * @brief static translation from match_type to adapter
			 * 
			 * @tparam mt objects::match_type
			 * @return adapter
			 */
			template<objects::match_type mt> constexpr auto& get()
			{
				dassert(false, "invalid specialization!!!"_u8);
			}
			template<> constexpr auto& get<objects::match_type::POLSL>()
			{
				return global_adapters::polsl;
			}
			template<> constexpr auto& get<objects::match_type::ORCID>()
			{
				return global_adapters::orcid;
			}
			template<> constexpr auto& get<objects::match_type::SCOPUS>()
			{
				return global_adapters::scopus;
			}
		}	 // namespace global_adapters
	}		 // namespace network
}	 // namespace core
