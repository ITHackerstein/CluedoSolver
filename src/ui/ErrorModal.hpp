#pragma once

#include <string>

/// \file ErrorModal.hpp
/// \brief The file that contains the definition of the \ref Cluedo::UI::ErrorModal class.

namespace Cluedo {

namespace UI {

/// \brief A modal used to show an error message.
class ErrorModal {
public:
	/// Construct the modal.
	explicit ErrorModal() = default;

	/// Shows the modal.
	void show();
	/// Sets the error message to be shown by the modal.
	void set_error_message(std::string_view error_message) { m_error_message = error_message; }

private:
	std::string m_error_message;
};

}

}
