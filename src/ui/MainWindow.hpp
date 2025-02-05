#pragma once

#include "../Solver.hpp"
#include "AddInformationModal.hpp"
#include "NewGameModal.hpp"
#include "PlayerDataModal.hpp"

/// \file MainWindow.hpp
/// \brief The file that contains the definition of the \ref Cluedo::UI::MainWindow class.

namespace Cluedo {

namespace UI {

/// \brief The main window of the application.
///
/// This class is responsible for showing the main window of the application.
/// It has a menubar with three sections:
/// * _Game_: contains the items _New_ (\ref Cluedo::UI::NewGameModal), _Add information (\ref Cluedo::UI::AddInformationModal)_ and _Player data_ (\ref Cluedo::UI::PlayerDataModal);
/// * _Settings_: contains the items _Language_ (allows to change the language of the application) and _Theme_ (allows to change the theme of the application);
/// * _About_: when clicking this item it will show brief information about the application.
///
/// The main window also contains two sections:
/// * the _Information history_ section that shows the information that was added to the solver;
/// * the _Solutions_ section that shows the solutions for the game with their respective probabilities.
/// \note This two sections will be available only after a game is created.
class MainWindow {
public:
	/// Constructs the window.
	explicit MainWindow();

	/// Shows the window.
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
