#include <antybiurokrata/libraries/orm/orm.h>
#include <antybiurokrata/types.hpp>

namespace core
{
	namespace orm
	{
		bool persons_extractor_t::visit(bgpolsl_repr_t* ptr)
		{
			dassert(ptr, "pointer cannot be nullptr!");

			const u16str_v affiliation{ ptr->affiliation };

			return true;
		}
	}
}

/*
idt: 0000140234
year: 2021
authors: Barglik Jerzy Sajkowski Maciej Smagór Adrian Stenzel Tomasz Czupała S. Jaros W. Koreń B. Kowalczyk K. Kulig M.
org_title: Projekt i budowa stanowiska laboratoryjnego do badania elementów ochrony przeciwporażeniowej z wykorzystaniem rzeczywistości mieszanej VR/AR
whole_title: 
e_doc: 
p_issn: 2544-2740
doi: 
e_issn: 2544-3771
affiliation: Barglik Jerzy 0000-0002-1994-3266 912576 054 RM Sajkowski Maciej 0000-0001-7953-1979 919111 104 RE Smagór Adrian 0000-0001-7667-6697 3926087 054 RM Stenzel Tomasz 0000-0002-5596-1330 919121 104 RE Czupała S. 000 000 Jaros W. 000 000 Koreń B. 000 000 Kowalczyk K. 000 000 Kulig M. 000 000
*/