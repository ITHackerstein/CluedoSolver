#include "Card.hpp"

#include <cassert>

namespace Cluedo {

std::string_view format_as(Card card) {
	using namespace std::literals;

	switch (card) {
#define _ENUMERATE_CARD(x) \
	case Card::x:            \
		return #x##sv;
		_ENUMERATE_CARDS
#undef _ENUMERATE_CARD
	default:
		assert(false);
		return ""sv;
	}
}

};
