#pragma once

#include "../Solver.hpp"

/// \file PlayerDataModal.hpp
/// \brief The file that contains the definition of the \ref Cluedo::UI::PlayerDataModal class.

namespace Cluedo {

namespace UI {

/// \brief A modal that shows the player data of a running game.
///
/// This modal will show the list containing the players with their names
/// and number of cards held.
class PlayerDataModal {
public:
	/// Constructs the modal.
	explicit PlayerDataModal() = default;

	/// Shows the modal.

	/// \param solver The solver that contains the game data.
	void show(Solver const& solver);
};

}

}
