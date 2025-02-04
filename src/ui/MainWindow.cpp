#include "MainWindow.hpp"

#include "../LanguageStrings.hpp"
#include "../utils/IconsFontAwesome.h"

#include <fmt/format.h>
#include <imgui.h>

namespace Cluedo {

namespace UI {

MainWindow::MainWindow()
  : m_new_game_modal([this](Solver&& solver) {
	  m_solver = std::move(solver);
	  m_solutions = m_solver->find_most_likely_solutions();
  })
  , m_add_information_modal([this](std::string&& information, Solver&& solver) {
	  m_information_history.emplace_back(std::move(information), std::move(solver));
	  m_solutions = m_solver->find_most_likely_solutions();
  }) {
}

void MainWindow::show_game_menu() {
	if (ImGui::BeginMenu(CSTR(LS("UI.Game")))) {
		if (ImGui::MenuItem(CSTR(LS("UI.New")), "CTRL+N")) {
			m_show_new_game_modal = true;
			m_new_game_modal.reset();
		}

		ImGui::Separator();

		if (ImGui::MenuItem(CSTR(LS("UI.AddInformation")), "CTRL+Enter", nullptr, m_solver.has_value())) {
			m_show_add_information_modal = true;
			m_add_information_modal.reset();
		}

		if (ImGui::MenuItem(CSTR(LS("UI.PlayerData")), nullptr, nullptr, m_solver.has_value())) {
			m_show_player_data_modal = true;
		}

		ImGui::EndMenu();
	}
}

void MainWindow::show_settings_menu() {
	if (ImGui::BeginMenu(CSTR(LS("UI.Settings")))) {
		if (ImGui::BeginMenu(CSTR(LS("UI.Language")))) {
			for (auto language : LanguageStrings::languages()) {
				if (ImGui::MenuItem(CSTR(language.name), nullptr, language.id == LanguageStrings::the().current_language_id())) {
					LanguageStrings::the().set_language(language.id);
				}
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(CSTR(LS("UI.Theme")))) {
			if (ImGui::MenuItem(CSTR(LS("UI.Light")), nullptr, m_style == Style::Light)) {
				m_style = Style::Light;
				ImGui::StyleColorsLight();
			}

			if (ImGui::MenuItem(CSTR(LS("UI.Dark")), nullptr, m_style == Style::Dark)) {
				m_style = Style::Dark;
				ImGui::StyleColorsDark();
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
}

void MainWindow::show_about_menu() {
	if (ImGui::MenuItem(CSTR(LS("UI.About")))) {
		ImGui::OpenPopup(CSTR(LS("UI.About")));
	}

	if (ImGui::BeginPopupModal(CSTR(LS("UI.About")), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted(CSTR(LS("UI.AboutText")));
		ImGui::Spacing();
		ImGui::TextDisabled("%s", CSTR(LS("UI.AuthorInfo")));
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		if (ImGui::Button(CSTR(LS("UI.Close")))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void MainWindow::show_menubar() {
	if (ImGui::BeginMenuBar()) {
		show_game_menu();
		show_settings_menu();
		show_about_menu();
		ImGui::EndMenuBar();
	}
}

void MainWindow::show_information_history_section() {
	ImGui::SeparatorText(CSTR(LS("UI.InformationHistory")));
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_DelayNormal)) {
		ImGui::SetTooltip("%s", CSTR(LS("UI.InformationHistoryTooltipText")));
	}

	if (ImGui::BeginChild("##information-history", { 0.0f, std::min(300.0f, ImGui::GetContentRegionAvail().y * 0.3f) })) {
		if (m_solver) {
			auto button_text = fmt::format("{} {}", ICON_FA_ARROW_ROTATE_LEFT, LS("UI.UndoLastInformation"));
			if (ImGui::Button(button_text.c_str())) {
				auto [_, solver] = std::move(m_information_history.back());
				m_information_history.pop_back();
				m_solver = std::move(solver);
				m_solutions = m_solver->find_most_likely_solutions();
			}

			if (ImGui::BeginListBox("##information-history-listbox", { -1, -1 })) {
				for (std::size_t i = 0; i < m_information_history.size(); ++i) {
					ImGui::PushID(i);
					ImGui::Selectable(m_information_history[i].first.c_str());
					ImGui::PopID();
				}
				ImGui::EndListBox();
			}
		}

		ImGui::EndChild();
	}
}

void MainWindow::show_solutions_section() {
	ImGui::SeparatorText(CSTR(LS("UI.Solutions")));
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_DelayNormal)) {
		ImGui::SetTooltip("%s", CSTR(LS("UI.SolutionsTooltipText")));
	}

	if (ImGui::BeginChild("##solutions")) {
		if (m_solver) {
			for (auto const& [solution, probability] : m_solutions) {
				auto [suspect, weapon, room] = solution;
				auto text = fmt::format("{}, {}, {}", suspect, weapon, room);
				ImGui::TextUnformatted(text.c_str());
				auto available_space = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(text.c_str()).x;
				if (available_space < 300.0f) {
					ImGui::SameLine();
					ImGui::ProgressBar(probability, { 0, 0 }, fmt::format("{:.2f}%", probability * 100).c_str());
				} else {
					ImGui::SameLine(ImGui::GetContentRegionAvail().x - 300.0f);
					ImGui::ProgressBar(probability, { 300.0f, 0 }, fmt::format("{:.2f}%", probability * 100).c_str());
				}
			}
		}

		ImGui::EndChild();
	}
}

void MainWindow::show() {
	m_show_new_game_modal = false;
	m_show_add_information_modal = false;
	m_show_player_data_modal = false;

	ImGui::SetNextWindowPos({ 0.0f, 0.0f });
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	if (ImGui::Begin("Cluedo Solver", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)) {
		show_menubar();
		show_information_history_section();
		show_solutions_section();
		ImGui::End();
	}
	ImGui::PopStyleVar();

	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_N)) {
		m_new_game_modal.reset();
		m_show_new_game_modal = true;
	}

	if (m_show_new_game_modal) {
		ImGui::OpenPopup(CSTR(LS("UI.NewGame")));
	}
	m_new_game_modal.show();

	if (m_solver) {
		if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Enter)) {
			m_add_information_modal.reset();
			m_show_add_information_modal = true;
		}

		if (m_show_add_information_modal) {
			ImGui::OpenPopup(CSTR(LS("UI.AddInformation")));
		}
		m_add_information_modal.show(*m_solver);

		if (m_show_player_data_modal) {
			ImGui::OpenPopup(CSTR(LS("UI.PlayerData")));
		}
		m_player_data_modal.show(*m_solver);
	}
}

}

}
