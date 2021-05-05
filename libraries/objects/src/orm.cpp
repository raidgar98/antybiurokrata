#include <antybiurokrata/libraries/patterns/seiralizer.hpp>
#include <antybiurokrata/libraries/orm/orm.h>
#include <antybiurokrata/types.hpp>

namespace core
{
	namespace orm
	{
		bool persons_extractor_t::visit(bgpolsl_repr_t *ptr)
		{
			dassert(ptr, "pointer cannot be nullptr!");

			const u16str_v affiliation{ptr->affiliation}; // alias
			int ticker{0};
			std::unique_ptr<person_t> person;
			const auto reset_person = [&person]{ person.reset(new person_t{}); };
			for (u16str_v part_of_affiliation : string_utils::split_words<u16str_v>{affiliation, u','})
			{
				if(part_of_affiliation.empty()) continue;
				const string_utils::split_words<u16str_v> splitter{part_of_affiliation, u' '};
				auto it = splitter.begin();
				reset_person();

				u16str_v v = (it == splitter.end() ? u"" : *it);
				const auto safely_move = [&]() -> bool 
				{
					it++;
					if(it == splitter.end()) return false;
					v = *it;
					return true;
				};

				if (polish_name_t::class_t::basic_validation(v)) (*person)().surname(v);
				else
				{
					log.warn() << "failed validation on surname: " << v << logger::endl;
					continue;
				}

				if(!safely_move()) continue;

				if (polish_name_t::class_t::basic_validation(v)) (*person)().name(v);
				else
				{
					log.warn() << "failed validation on name: " << v << logger::endl;
					continue;
				}

				if(!safely_move()) continue;

				if (orcid_t::class_t::is_valid(v)) (*person)().orcid(orcid_t::class_t::from_string(v));
				else
				{
					log.warn() << "failed validation on orcid: " << v << logger::endl;
					continue;
				}

				auto pair = this->persons.emplace(std::move(*person));
				if(pair.second) log.info() << "successfully added new author: " << patterns::serial::pretty_print{*pair.first} << logger::endl;
				if (current_publication) pair.first->val.publictions().push_back(current_publication);

			}

			return true;
		}

		bool publications_extractor_t::visit(bgpolsl_repr_t *ptr)
		{
			dassert(ptr, "pointer cannot be nullptr!");

			publication_storage_t spub{new publication_t{}};
			publication_t &pub = *spub;
			person_visitor.current_publication.reset();

			using objects::detail::id_type;
			const auto fill_id = [&](const u16str_v view, const id_type it) { if(!view.empty()) pub().ids()[it] = view; };
			fill_id(ptr->idt, id_type::IDT);
			fill_id(ptr->doi, id_type::DOI);
			fill_id(ptr->e_issn, id_type::EISSN);
			fill_id(ptr->p_issn, id_type::PISSN);

			if (pub().ids().empty())
				return false;

			if (ptr->year.empty())
				return false;
			else
				pub().year(std::stoi(get_conversion_engine().to_bytes(ptr->year)));

			if (ptr->org_title.empty() && ptr->whole_title.empty())
				return false;
			else
			{
				if (ptr->org_title.empty())
					pub().title(ptr->whole_title);
				else if (ptr->whole_title.empty())
					pub().title(ptr->org_title);
				else
				{
					if (demangler<>::is_polish(ptr->org_title))
						pub().polish_title(ptr->org_title);
					else
						pub().title(ptr->org_title);

					if (pub().polish_title().empty() && demangler<>::is_polish(ptr->whole_title))
						pub().polish_title(ptr->whole_title);
					else if (pub().title().empty())
						pub().title(ptr->whole_title);
				}
			}

			publications.push_back(spub);
			person_visitor.current_publication = spub;
			person_visitor.visit(ptr);

			return true;
		}

	}
}