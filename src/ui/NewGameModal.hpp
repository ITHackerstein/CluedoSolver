#pragma once

#include "../Solver.hpp"
#include "ErrorModal.hpp"

#include <functional>
#include <vector>

namespace Cluedo {

namespace UI {

class NewGameModal {
public:
	explicit NewGameModal(std::function<void(Solver&&)> on_solver_created_callback)
	  : m_on_solver_created_callback(on_solver_created_callback) {}

	void reset();
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
