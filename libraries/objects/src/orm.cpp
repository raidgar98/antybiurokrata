#include <antybiurokrata/libraries/patterns/seiralizer.hpp>
#include <antybiurokrata/libraries/orm/orm.h>
#include <antybiurokrata/types.hpp>

namespace core
{
	namespace orm
	{
		bool persons_extractor_t::visit(bgpolsl_repr_t* ptr)
		{
			check_nullptr{ptr};
			shared_person_t result;

			const u16str_v affiliation{ptr->affiliation};	// alias
			auto& person				= result().data();		// alias
			const auto reset_person = [&person] { person.reset(new person_t{}); };
			for(u16str_v part_of_affiliation: string_utils::split_words<u16str_v>{affiliation, u','})
			{
				if(part_of_affiliation.empty()) continue;
				const string_utils::split_words<u16str_v> splitter{part_of_affiliation, u' '};
				auto it = splitter.begin();
				reset_person();

				u16str_v v				  = (it == splitter.end() ? u"" : *it);
				const auto safely_move = [&]() -> bool {
					it++;
					if(it == splitter.end()) return false;
					v = *it;
					return true;
				};

				if(polish_name_t::value_t::validate(v)) (*person)().surname(v);
				else
				{
					log.warn() << "failed validation on surname: " << v << logger::endl;
					continue;
				}

				if(!safely_move()) continue;

				if(polish_name_t::value_t::validate(v)) (*person)().name(v);
				else
				{
					log.warn() << "failed validation on name: " << v << logger::endl;
					continue;
				}

				if(!safely_move()) continue;

				if(orcid_t::value_t::is_valid_orcid_string(v))
					(*person)().orcid(orcid_t::value_t::from_string(v));
				else
				{
					log.warn() << "failed validation on orcid: " << v << logger::endl;
					continue;
				}

				auto pair = this->persons->insert(result);
				if(pair.second)
					log.info() << "successfully added new author: "
								  << patterns::serial::pretty_print{(*(*pair.first)())} << logger::endl;
				if(current_publication())
					(*(*pair.first)())().publictions()()->insert(current_publication().data());
			}

			return true;
		}

		bool persons_extractor_t::visit(json_repr_t* ptr)
		{
			check_nullptr{ptr};
			shared_person_t result;
			auto& person = result().data();
			person_t& pp{*person};

			if(ptr->orcid.empty()
				|| !objects::detail::detail_orcid_t::is_valid_orcid_string(ptr->orcid))
				return false;
			else
				pp().orcid() = objects::detail::detail_orcid_t::from_string(ptr->orcid);

			if(!pp().orcid()().is_valid_orcid()) return false;

			auto found = persons->find(person);
			dassert(found != persons->end(), "unknown person for given orcid!"_u8);

			(*(*found)())().publictions()()->insert(current_publication());
			return true;
		}

		bool publications_extractor_t::visit(bgpolsl_repr_t* ptr)
		{
			check_nullptr{ptr};

			shared_publication_t spub{new publication_t{}};
			publication_t& pub = *spub();
			person_visitor.current_publication().data().reset();

			using objects::detail::id_type;
			const auto fill_id = [&](const u16str_v view, const id_type it) {
				if(!view.empty()) pub().ids()().data()[it] = view;
			};
			fill_id(ptr->idt, id_type::IDT);
			fill_id(ptr->doi, id_type::DOI);
			fill_id(ptr->e_issn, id_type::EISSN);
			fill_id(ptr->p_issn, id_type::PISSN);

			if(pub().ids()()->empty()) return false;

			if(ptr->year.empty()) return false;
			else
				pub().year(std::stoi(get_conversion_engine().to_bytes(ptr->year)));

			if(ptr->org_title.empty() && ptr->whole_title.empty()) return false;
			else
			{
				if(ptr->org_title.empty()) pub().title(ptr->whole_title);
				else if(ptr->whole_title.empty())
					pub().title(ptr->org_title);
				else
				{
					if(demangler<>::is_polish(ptr->org_title)) pub().polish_title(ptr->org_title);
					else
						pub().title(ptr->org_title);

					if(pub().polish_title()()->empty() && demangler<>::is_polish(ptr->whole_title))
						pub().polish_title(ptr->whole_title);
					else if(pub().title()()->empty())
						pub().title(ptr->whole_title);
				}
			}

			demangler<>::sanitize(pub().title()().data());
			demangler<>::sanitize(pub().polish_title()().data());

			publications.push_back(spub);
			person_visitor.current_publication = spub;
			ptr->accept(&person_visitor);

			return true;
		}

		bool publications_extractor_t::visit(json_repr_t* ptr)
		{
			check_nullptr{ptr};

			shared_publication_t spub{new publication_t{}};
			publication_t& pub = *spub();
			person_visitor.current_publication().data().reset();

			if(ptr->year.empty())
			{
				log.warn() << "failed validation because of empty year" << logger::endl;
				return false;
			}
			else
				pub().year(std::stoi(get_conversion_engine().to_bytes(ptr->year)));

			if(ptr->title.empty())
			{
				log.warn() << "failed validation because of empty title" << logger::endl;
				return false;
			}
			else
				pub().title(ptr->title);

			if(!ptr->translated_title.empty())
			{
				pub().polish_title(ptr->translated_title);
				if(demangler<>::is_polish(pub().title()())
					&& !demangler<>::is_polish(pub().polish_title()().data()))
					std::swap(pub().title(), pub().polish_title());

				demangler<>::sanitize(pub().polish_title()().data());
			}

			demangler<>::sanitize(pub().title()().data());

			for(const auto& pair: ptr->ids)
			{
				const objects::id_type id = objects::detail::id_type_stringinizer::get(pair.first);

				if(id == objects::id_type::NOT_FOUND) continue;
				auto exists_pair = pub().ids()()->find(id);
				if(exists_pair == pub().ids()()->end()) pub().ids()().data()[id] = pair.second;
			}

			if(pub().ids()()->empty())
			{
				log.warn() << "json input has no ids" << logger::endl;
				return false;
			}

			publications.push_back(spub);
			log.info() << "succefully added new publications" << logger::endl;

			person_visitor.current_publication = spub;

			if(!ptr->accept(&person_visitor))
				log.warn() << "adding person with orcid: `" << ptr->orcid << "` failed" << logger::endl;
			return true;
		}

		void persons_extractor_t::shallow_copy_persons(const persons_extractor_t& input,
																	  persons_extractor_t& output)
		{
			for(const auto& person: *input.persons)
			{
				shared_person_t np{new person_t{}};
				(*np())().name		= (*person())().name;
				(*np())().surname = (*person())().surname;
				(*np())().orcid	= (*person())().orcid;
				auto pair			= output.persons->insert(np().data());
				if(pair.second) (*(*pair.first)())().publictions()()->clear();
			}
		}
	}	 // namespace orm
}	 // namespace core
