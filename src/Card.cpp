#include "Card.hpp"

#include <cassert>

#include "LanguageStrings.hpp"

namespace Cluedo {

std::string_view format_as(CardCategory category) {
	using namespace std::literals;

	switch (category) {
	case CardCategory::Suspect:
		return Cluedo::LanguageStrings::the().get_string("CardCategory.Suspect"sv);
	case CardCategory::Weapon:
		return Cluedo::LanguageStrings::the().get_string("CardCategory.Weapon"sv);
	case CardCategory::Room:
		return Cluedo::LanguageStrings::the().get_string("CardCategory.Room"sv);
	default:
		assert(false);
		return ""sv;
	}
}

std::string_view format_as(Card card) {
	using namespace std::literals;

	switch (card) {
#define _ENUMERATE_CARD(x) \
	case Card::x:            \
		return Cluedo::LanguageStrings::the().get_string("Card." #x##sv);
		_ENUMERATE_CARDS
#undef _ENUMERATE_CARD
	default:
		assert(false);
		return ""sv;
	}
}

};
