#pragma once

#include <string_view>

namespace Cluedo {

enum class Error {
	InvalidNumberOfPlayers,
	InvalidNumberOfCards,
	SuggestingPlayerEqualToRespondingPlayer
};

std::string_view format_as(Error error);

}
