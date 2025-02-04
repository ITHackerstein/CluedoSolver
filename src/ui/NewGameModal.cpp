#include "NewGameModal.hpp"

#include "../LanguageStrings.hpp"

#include <fmt/format.h>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace Cluedo {

namespace UI {

void NewGameModal::reset() {
	m_player_count = Solver::MAX_PLAYER_COUNT;
	m_players.resize(Solver::MAX_PLAYER_COUNT);

	for (auto& player : m_players) {
		player.name.clear();
		player.card_count = (CardUtils::CARD_COUNT - Solver::SOLUTION_CARD_COUNT) / Solver::MAX_PLAYER_COUNT;
	}
}

void NewGameModal::show() {
	if (ImGui::BeginPopupModal(CSTR(LS("UI.NewGame")), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		show_number_of_players_input();
		show_players_section();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		show_buttons();

		ImGui::EndPopup();
	}
}

void NewGameModal::show_number_of_players_input() {
	static std::size_t u64_one = 1;
	if (ImGui::InputScalar(CSTR(LS("UI.NumberOfPlayers")), ImGuiDataType_U64, &m_player_count, &u64_one)) {
		m_player_count = std::max(std::min(m_player_count, Solver::MAX_PLAYER_COUNT), Solver::MIN_PLAYER_COUNT);
		if (m_players.size() != m_player_count) {
			m_players.resize(m_player_count);

			auto available_cards = CardUtils::CARD_COUNT - Solver::SOLUTION_CARD_COUNT;
			auto cards_per_player = available_cards / m_player_count;
			auto remaining_cards = available_cards % m_player_count;

			for (std::size_t i = 0; i < m_player_count; ++i) {
				auto& player = m_players[i];
				player.name.clear();
				player.card_count = cards_per_player;
				if (remaining_cards > 0) {
					++player.card_count;
					--remaining_cards;
				}
			}
		}
	}
}

void NewGameModal::show_players_section() {
	ImGui::SeparatorText(CSTR(LS("UI.Players")));
	for (size_t i = 0; i < m_players.size(); ++i) {
		ImGui::PushID(i);
		ImGui::InputTextWithHint("##name", fmt::format("{} {}", LS("UI.Player"), i + 1).c_str(), &m_players[i].name);
		ImGui::SameLine();

		if (ImGui::ArrowButton("##card_count_decrease", ImGuiDir_Left)) {
			m_players[i].card_count = std::max(m_players[i].card_count - 1, 1uz);
		}

		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::Text("%zu", m_players[i].card_count);

		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		if (ImGui::ArrowButton("##card_count_increase", ImGuiDir_Right)) {
			m_players[i].card_count = std::max(m_players[i].card_count + 1, 1uz);
		}

		ImGui::SameLine();
		ImGui::TextUnformatted(CSTR(LS("UI.Cards")));
		ImGui::PopID();
	}
}

void NewGameModal::show_buttons() {
	if (ImGui::Button(CSTR(LS("UI.Ok")))) {
		auto maybe_solver = Solver::create(m_players);
		if (maybe_solver.is_error()) {
			m_error_modal.set_error_message(fmt::format("{}: {}!", LS("UI.ErrorWhileCreatingGame"), maybe_solver.release_error()));
			ImGui::OpenPopup(CSTR(LS("UI.Error")));
		} else {
			m_error_modal.set_error_message("");
			m_on_solver_created_callback(maybe_solver.release_value());
			ImGui::CloseCurrentPopup();
		}
	}

	ImGui::SameLine();
	if (ImGui::Button(CSTR(LS("UI.Cancel")))) {
		ImGui::CloseCurrentPopup();
	}

	m_error_modal.show();
}

}

}
