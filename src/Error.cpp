#include "Error.hpp"

#include <cassert>

#include "Strings.hpp"

namespace Cluedo {

std::string_view format_as(Error error) {
	using namespace std::literals;

	switch (error) {
#define _ENUMERATE_ERROR(x) \
	case Error::x: \
		return Cluedo::Strings::the().get_string("Error."#x ## sv);
	_ENUMERATE_ERRORS
#undef _ENUMERATE_ERROR
	}

	assert(false);
	return ""sv;
}

}
