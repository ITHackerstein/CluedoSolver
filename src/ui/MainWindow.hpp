#pragma once

#include "../Solver.hpp"
#include "AddInformationModal.hpp"
#include "NewGameModal.hpp"
#include "PlayerDataModal.hpp"

namespace Cluedo {

namespace UI {

class MainWindow {
public:
	explicit MainWindow();

	void show();

private:
	void show_game_menu();
	void show_settings_menu();
	void show_about_menu();
	void show_menubar();

	void show_information_history_section();
	void show_solutions_section();

	enum class Style {
		Light,
		Dark
	};

	Style m_style { Style::Dark };

	std::optional<Solver> m_solver;
	std::vector<std::pair<std::string, Solver>> m_information_history;
	std::vector<Solver::SolutionProbabilityPair> m_solutions;

	bool m_show_new_game_modal { false };
	NewGameModal m_new_game_modal;

	bool m_show_add_information_modal { false };
	AddInformationModal m_add_information_modal;

	bool m_show_player_data_modal { false };
	PlayerDataModal m_player_data_modal;
};

}

}
