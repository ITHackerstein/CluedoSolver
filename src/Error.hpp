#pragma once

#include <string_view>

/// \file Error.hpp
/// \brief This file contains the list of errors that can occur in the application.

namespace Cluedo {

/// \def _ENUMERATE_ERRORS
/// \brief A macro that enumerates the errors that can occur in the application.
#define _ENUMERATE_ERRORS                  \
	_ENUMERATE_ERROR(InvalidNumberOfPlayers) \
	_ENUMERATE_ERROR(InvalidNumberOfCards)   \
	_ENUMERATE_ERROR(InvalidInformation)

/// \enum Error
/// The list of errors that can occur in the application.
enum class Error {
#define _ENUMERATE_ERROR(x) x,
	_ENUMERATE_ERRORS
#undef _ENUMERATE_ERROR
};

/// Formats the error as a string.
/// \note This function is meant to be used by the
/// <a href="https://github.com/fmtlib/fmt">{fmt}</a> library to format the
/// errors correctly.
std::string_view format_as(Error error);

}
