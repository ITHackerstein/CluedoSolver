#pragma once

#include "../Solver.hpp"
#include "ErrorModal.hpp"

#include <functional>
#include <vector>

/// \file NewGameModal.hpp
/// \brief The file that contains the definition of the \ref Cluedo::UI::NewGameModal class.

namespace Cluedo {

namespace UI {

/// \brief A modal used to create a new game.
///
/// This modal will ask the user to type the number of players and the of the players (their names and number of cards).
class NewGameModal {
public:
	/// Construct the modal.
	///
	/// \param on_solver_created_callback The function to be called when the \ref Cluedo::Solver for that game is created.
	explicit NewGameModal(std::function<void(Solver&&)> on_solver_created_callback)
	  : m_on_solver_created_callback(on_solver_created_callback) {}

	/// Resets the modal data.
	/// \note Used when the modal has to be opened, so that the old data is lost.
	void reset();
	/// Shows the modal.
	void show();

private:
	void show_number_of_players_input();
	void show_players_section();
	void show_buttons();

	std::size_t m_player_count;
	std::vector<PlayerData> m_players;
	std::function<void(Solver&&)> m_on_solver_created_callback;
	ErrorModal m_error_modal;
};

}

}
