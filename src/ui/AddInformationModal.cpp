#include "AddInformationModal.hpp"

#include "../LanguageStrings.hpp"

#include <fmt/format.h>
#include <imgui.h>

namespace Cluedo {

namespace UI {

void show_player_combobox(char const* id, Solver const& solver, size_t& selection) {
	ImGui::PushID(id);
	if (ImGui::BeginCombo("##", solver.player(selection).name().c_str(), ImGuiComboFlags_WidthFitPreview)) {
		for (size_t i = 0; i < solver.player_count(); ++i) {
			bool is_selected = selection == i;
			if (ImGui::Selectable(solver.player(i).name().c_str(), is_selected)) {
				selection = i;
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopID();
}

void show_optional_player_combobox(char const* id, Solver const& solver, size_t player_to_exclude_index, std::optional<size_t>& selection) {
	ImGui::PushID(id);
	if (selection == player_to_exclude_index) {
		selection.reset();
	}

	if (ImGui::BeginCombo("##", selection.has_value() ? solver.player(*selection).name().c_str() : CSTR(LS("UI.NoOne")), ImGuiComboFlags_WidthFitPreview)) {
		if (ImGui::Selectable(CSTR(LS("UI.NoOne")), !selection.has_value())) {
			selection.reset();
		}

		if (!selection.has_value()) {
			ImGui::SetItemDefaultFocus();
		}

		for (size_t i = 0; i < solver.player_count(); ++i) {
			if (player_to_exclude_index == i) {
				continue;
			}

			bool is_selected = *selection == i;
			if (ImGui::Selectable(solver.player(i).name().c_str(), is_selected)) {
				selection = i;
			}

			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopID();
}

template<typename CardIterator>
void show_card_combobox(char const* id, CardIterator cards_iterator, Card& selection) {
	ImGui::PushID(id);
	if (ImGui::BeginCombo("##", fmt::format("{}", selection).c_str(), ImGuiComboFlags_WidthFitPreview)) {
		for (auto card : cards_iterator) {
			bool is_selected = selection == card;
			if (ImGui::Selectable(fmt::format("{}", card).c_str(), is_selected)) {
				selection = card;
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopID();
}

void AddInformationModal::PlayerCardStateTab::reset() {
	m_player_index = 0;
	m_card = *CardUtils::cards().begin();
	m_card_state = true;
}

void AddInformationModal::PlayerCardStateTab::show(AddInformationModal::Tab& tab, Solver& solver) {
	if (ImGui::BeginTabItem(CSTR(LS("UI.PlayerHasHasntGotACard")))) {
		tab = AddInformationModal::Tab::PlayerCardState;

		show_player_combobox("player-combobox", solver, m_player_index);

		ImGui::SameLine();
		ImGui::Checkbox(CSTR(LS(m_card_state ? "UI.HasGot" : "UI.HasntGot")), &m_card_state);

		ImGui::SameLine();
		show_card_combobox("card-combobox", CardUtils::cards(), m_card);

		ImGui::EndTabItem();
	}
}

void AddInformationModal::PlayerCardStateTab::learn(Solver& solver) {
	solver.learn_player_card_state(m_player_index, m_card, m_card_state);
}

std::string AddInformationModal::PlayerCardStateTab::compute_information_string(Solver const& solver) {
	return fmt::format(
	  "{} {} {}",
	  solver.player(m_player_index).name(),
	  LS(m_card_state ? "UI.HasGot" : "UI.HasntGot"),
	  m_card
	);
}

void AddInformationModal::SuggestionTab::reset() {
	m_suggestion.suggesting_player_index = 0;
	m_suggestion.suspect = *CardUtils::cards_per_category(CardCategory::Suspect).begin();
	m_suggestion.weapon = *CardUtils::cards_per_category(CardCategory::Weapon).begin();
	m_suggestion.room = *CardUtils::cards_per_category(CardCategory::Room).begin();
	m_suggestion.responding_player_index.reset();
	m_suggestion.response_card.reset();
}

void AddInformationModal::SuggestionTab::show(Tab& tab, Solver& solver) {
	if (ImGui::BeginTabItem(CSTR(LS("UI.PlayerMadeASuggestion")))) {
		tab = AddInformationModal::Tab::Suggestion;

		show_player_combobox("suggesting-player-combobox", solver, m_suggestion.suggesting_player_index);

		ImGui::SameLine();
		ImGui::TextUnformatted(CSTR(LS("UI.Suggested")));

		ImGui::SameLine();
		show_card_combobox("suspect-combobox", CardUtils::cards_per_category(CardCategory::Suspect), m_suggestion.suspect);

		ImGui::SameLine();
		show_card_combobox("weapon-combobox", CardUtils::cards_per_category(CardCategory::Weapon), m_suggestion.weapon);

		ImGui::SameLine();
		show_card_combobox("room-combobox", CardUtils::cards_per_category(CardCategory::Room), m_suggestion.room);

		show_optional_player_combobox("responding-player-combobox", solver, m_suggestion.suggesting_player_index, m_suggestion.responding_player_index);

		ImGui::SameLine();
		ImGui::TextUnformatted(CSTR(LS(m_suggestion.responding_player_index ? "UI.RespondedWith" : "UI.Responded")));

		if (m_suggestion.responding_player_index) {
			ImGui::SameLine();
			ImGui::PushID("response-card-combobox");
			if (ImGui::BeginCombo("##", m_suggestion.response_card ? fmt::format("{}", *m_suggestion.response_card).c_str() : CSTR(LS("UI.Unknown")), ImGuiComboFlags_WidthFitPreview)) {
				if (ImGui::Selectable(CSTR(LS("UI.Unknown")), !m_suggestion.response_card)) {
					m_suggestion.response_card.reset();
				}

				if (!m_suggestion.response_card) {
					ImGui::SetItemDefaultFocus();
				}

				for (auto card : { m_suggestion.suspect, m_suggestion.weapon, m_suggestion.room }) {
					bool is_selected = m_suggestion.response_card == card;
					if (ImGui::Selectable(fmt::format("{}", card).c_str(), is_selected)) {
						m_suggestion.response_card = card;
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
			ImGui::PopID();
		}

		ImGui::EndTabItem();
	}
}

void AddInformationModal::SuggestionTab::learn(Solver& solver) {
	solver.learn_from_suggestion(m_suggestion);
}

std::string AddInformationModal::SuggestionTab::compute_information_string(Solver const& solver) {
	std::string response = std::string { LS("UI.NoOneResponded") };
	if (m_suggestion.responding_player_index) {
		auto const& responding_player_name = solver.player(*m_suggestion.responding_player_index).name();
		if (m_suggestion.response_card) {
			response = fmt::format("{} {} {}", responding_player_name, LS("UI.RespondedWith"), *m_suggestion.response_card);
		} else {
			response = fmt::format("{} {}", responding_player_name, LS("UI.Responded"));
		}
	}

	return fmt::format(
	  "{} {} {}, {}, {} {} {}",
	  solver.player(m_suggestion.suggesting_player_index).name(),
	  LS("UI.Suggested"),
	  m_suggestion.suspect, m_suggestion.weapon, m_suggestion.room,
	  LS("UI.And"),
	  response
	);
}

void AddInformationModal::show_buttons(Solver& solver) {
	if (ImGui::Button(CSTR(LS("UI.Learn")))) {
		auto old_solver = solver;
		std::string information;

		if (m_selected_tab == Tab::PlayerCardState) {
			m_player_card_state_tab.learn(solver);
			information = m_player_card_state_tab.compute_information_string(solver);
		} else if (m_selected_tab == Tab::Suggestion) {
			m_suggestion_tab.learn(solver);
			information = m_suggestion_tab.compute_information_string(solver);
		}

		if (!solver.are_constraints_satisfied()) {
			solver = old_solver;
			m_error_modal.set_error_message(fmt::format("{}: {}!", LS("UI.ErrorWhileLearningNewInformation"), Error::InvalidInformation));
			ImGui::OpenPopup(CSTR(LS("UI.Error")));
		} else {
			m_error_modal.set_error_message("");
			m_on_learn_callback(std::move(information), std::move(old_solver));
			ImGui::CloseCurrentPopup();
		}
	}

	ImGui::SameLine();
	if (ImGui::Button(CSTR(LS("UI.Cancel")))) {
		ImGui::CloseCurrentPopup();
	}
}

void AddInformationModal::reset() {
	m_selected_tab = Tab::PlayerCardState;
	m_player_card_state_tab.reset();
	m_suggestion_tab.reset();
}

void AddInformationModal::show(Solver& solver) {
	if (ImGui::BeginPopupModal(CSTR(LS("UI.AddInformation")), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		if (ImGui::BeginTabBar("##information-type-tab-bar", ImGuiTabBarFlags_None)) {
			m_player_card_state_tab.show(m_selected_tab, solver);
			m_suggestion_tab.show(m_selected_tab, solver);

			ImGui::EndTabBar();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		show_buttons(solver);
		m_error_modal.show();

		ImGui::EndPopup();
	}
}

}

}
