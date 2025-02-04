#pragma once

#include <string>

namespace Cluedo {

namespace UI {

class ErrorModal {
public:
	explicit ErrorModal() = default;

	void show();
	void set_error_message(std::string_view error_message) { m_error_message = error_message; }

private:
	std::string m_error_message;
};

}

}
