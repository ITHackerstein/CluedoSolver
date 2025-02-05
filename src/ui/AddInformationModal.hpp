#pragma once

#include <functional>

#include "../Solver.hpp"
#include "ErrorModal.hpp"

/// \file AddInformationModal.hpp
/// \brief The file that contains the definition of the \ref Cluedo::UI::AddInformationModal class.

namespace Cluedo {

namespace UI {

/// \brief A modal used to add information to the solver.
///
/// This modal is split in two tabs as there are two types of information that
//// can be learnt during a game:
/// * _Player card state_: a player has a card or not
/// * _Suggestion_: a player made a suggestion
/// The first tab will prompt the user to input the player who has the card or
/// not and the card in question. The second tab will prompt the user to input
/// the suggestion made.
class AddInformationModal {
public:
	/// Constructs the modal.
	///
	/// \param on_learn_callback The function to be called when the information is
	/// learnt succesfully.
	explicit AddInformationModal(std::function<void(std::string&&, Solver&&)> on_learn_callback)
	  : m_on_learn_callback(on_learn_callback) {}

	/// Resets the modal data.
	/// \note Used when the modal has to be opened, so that the old data is lost.
	void reset();
	/// Shows the modal.
	/// \param solver The solver that contains the game data.
	void show(Solver& solver);

private:
	void show_buttons(Solver&);

	enum class Tab {
		PlayerCardState,
		Suggestion
	};

	class PlayerCardStateTab {
	public:
		void reset();
		void show(Tab&, Solver&);

		void learn(Solver&);
		std::string compute_information_string(Solver const&);

	private:
		size_t m_player_index;
		Card m_card;
		bool m_card_state;
	};

	class SuggestionTab {
	public:
		void reset();
		void show(Tab&, Solver&);

		void learn(Solver&);
		std::string compute_information_string(Solver const&);

	private:
		Solver::Suggestion m_suggestion;
	};

	Tab m_selected_tab;
	PlayerCardStateTab m_player_card_state_tab;
	SuggestionTab m_suggestion_tab;
	std::function<void(std::string&&, Solver&&)> m_on_learn_callback;
	ErrorModal m_error_modal;
};

}

}
