#pragma once

#include <string_view>

namespace Cluedo {

#define _ENUMERATE_ERRORS                  \
	_ENUMERATE_ERROR(InvalidNumberOfPlayers) \
	_ENUMERATE_ERROR(InvalidNumberOfCards)   \
	_ENUMERATE_ERROR(SuggestingPlayerEqualToRespondingPlayer)

enum class Error {
#define _ENUMERATE_ERROR(x) x,
	_ENUMERATE_ERRORS
#undef _ENUMERATE_ERROR
};

std::string_view format_as(Error error);

}
