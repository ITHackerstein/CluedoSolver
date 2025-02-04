#pragma once

#include <functional>

#include "../Solver.hpp"
#include "ErrorModal.hpp"

namespace Cluedo {

namespace UI {

class AddInformationModal {
public:
	explicit AddInformationModal(std::function<void(std::string&&, Solver&&)> on_learn_callback)
	  : m_on_learn_callback(on_learn_callback) {}

	void reset();
	void show(Solver&);

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
