#include "Error.hpp"

#include <cassert>

namespace Cluedo {

std::string_view format_as(Error error) {
	using namespace std::literals;

	switch (error) {
	case Error::InvalidNumberOfPlayers:
		return "invalid number of players!"sv;
	case Error::InvalidNumberOfCards:
		return "invalid number of cards!"sv;
	default:
		assert(false);
		return ""sv;
	}
}

}
