#include "LanguageStrings.hpp"
#include "Solver.hpp"
#include "fonts/fonts.cpp"
#include "utils/IconsFontAwesome.h"

#include <fmt/color.h>
#include <fmt/core.h>
#include <string_view>
#include <vector>

#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#	include <SDL_opengles2.h>
#else
#	include <SDL_opengl.h>
#endif
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <imgui_stdlib.h>

#define S(key) (Cluedo::LanguageStrings::the().get_string((key)))
#define CSTR(s) (std::string { (s) }.c_str())

using namespace std::literals;

size_t const u64_one = 1;

struct UIData {
	bool running { true };
	int style { 1 };

	// New game modal data
	bool show_new_game_modal { false };
	size_t new_game_player_count;
	std::vector<Cluedo::PlayerData> new_game_players_data;

	// Add information modal data
	bool show_add_information_modal { false };
	size_t selected_tab_index;
	// Has card tab
	size_t first_tab_selected_player;
	bool first_tab_is_has_card_checked;
	Cluedo::Card first_tab_selected_card;
	// Suggestion tab
	Cluedo::Solver::Suggestion second_tab_suggestion;
	std::optional<Cluedo::CardCategory> second_tab_response_card_category;

	// Player data modal data
	bool show_player_data_modal { false };

	// Error modal data
	std::string error_message;

	// Current game solver
	std::optional<Cluedo::Solver> solver;

	// Information history section data
	std::vector<std::pair<std::string, Cluedo::Solver>> information_history;

	// Solutions section data
	std::vector<Cluedo::Solver::SolutionProbabilityPair> solutions;

	void initialize_new_game_data() {
		new_game_player_count = Cluedo::Solver::MAX_PLAYER_COUNT;
		new_game_players_data.resize(Cluedo::Solver::MAX_PLAYER_COUNT);
		for (auto& player_data : new_game_players_data) {
			player_data.name.clear();
			player_data.n_cards = Cluedo::CardUtils::CARD_COUNT / Cluedo::Solver::MAX_PLAYER_COUNT;
		}
	}

	void initialize_add_information_data() {
		selected_tab_index = 0;
		first_tab_selected_player = 0;
		first_tab_is_has_card_checked = true;
		first_tab_selected_card = static_cast<Cluedo::Card>(0);
		second_tab_suggestion.suggesting_player_index = 0;
		second_tab_suggestion.suspect = static_cast<Cluedo::Card>(0);
		second_tab_suggestion.weapon = static_cast<Cluedo::Card>(Cluedo::CardUtils::cards_per_category(Cluedo::CardCategory::Suspect).count());
		second_tab_suggestion.room = static_cast<Cluedo::Card>(Cluedo::CardUtils::cards_per_category(Cluedo::CardCategory::Suspect).count() + Cluedo::CardUtils::cards_per_category(Cluedo::CardCategory::Weapon).count());
		second_tab_suggestion.responding_player_index.reset();
		second_tab_suggestion.response_card.reset();
	}
};

template<typename T>
T clamp(T value, T min, T max) {
	return value < min ? min : (value > max ? max : value);
}

void show_error_modal(UIData& ui_data) {
	if (ImGui::BeginPopupModal(CSTR(S("UI.Error")), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::PushStyleColor(ImGuiCol_Text, 0xCA0B00FF);
		ImGui::AlignTextToFramePadding();
		ImGui::TextColored({ 0.79f, 0.04f, 0.00f, 1.00f }, ICON_FA_TRIANGLE_EXCLAMATION);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::Text("%s", ui_data.error_message.c_str());
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		if (ImGui::Button(CSTR(S("UI.Ok")))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void show_menubar(UIData& ui_data) {
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu(CSTR(S("UI.Game")))) {
			if (ImGui::MenuItem(CSTR(S("UI.New")), "CTRL+N")) {
				ui_data.show_new_game_modal = true;
				ui_data.initialize_new_game_data();
			}

			ImGui::Separator();

			if (ImGui::MenuItem(CSTR(S("UI.AddInformation")), "CTRL+Enter", nullptr, ui_data.solver.has_value())) {
				ui_data.show_add_information_modal = true;
				ui_data.initialize_add_information_data();
			}

			if (ImGui::MenuItem(CSTR(S("UI.PlayerData")), nullptr, nullptr, ui_data.solver.has_value())) {
				ui_data.show_player_data_modal = true;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(CSTR(S("UI.Settings")))) {
			if (ImGui::BeginMenu(CSTR(S("UI.Language")))) {
				for (auto language : Cluedo::LanguageStrings::languages()) {
					if (ImGui::MenuItem(std::string { language.name }.c_str(), nullptr, language.id == Cluedo::LanguageStrings::the().current_language_id())) {
						Cluedo::LanguageStrings::the().set_language(language.id);
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(CSTR(S("UI.Theme")))) {
				if (ImGui::MenuItem(CSTR(S("UI.Light")), nullptr, ui_data.style == 0)) {
					ui_data.style = 0;
					ImGui::StyleColorsLight();
				}

				if (ImGui::MenuItem(CSTR(S("UI.Dark")), nullptr, ui_data.style == 1)) {
					ui_data.style = 1;
					ImGui::StyleColorsDark();
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::MenuItem(CSTR(S("UI.About")))) {
			ImGui::OpenPopup(CSTR(S("UI.About")));
		}

		if (ImGui::BeginPopupModal(CSTR(S("UI.About")), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::TextUnformatted(CSTR(S("UI.AboutText")));
			ImGui::Spacing();
			ImGui::TextDisabled("%s", CSTR(S("UI.AuthorInfo")));
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			if (ImGui::Button(CSTR(S("UI.Close")))) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::EndMenuBar();
	}
}

void show_information_history_section(UIData& ui_data) {
	if (ImGui::BeginChild("##information-history", { 0.0f, std::min(300.0f, ImGui::GetContentRegionAvail().y * 0.3f) })) {
		if (ui_data.solver) {
			auto button_text = fmt::format("{} {}", ICON_FA_ARROW_ROTATE_LEFT, S("UI.UndoLastInformation"));
			if (ImGui::Button(button_text.c_str())) {
				auto [_, solver] = std::move(ui_data.information_history.back());
				ui_data.information_history.pop_back();
				ui_data.solver = solver;
				ui_data.solutions = ui_data.solver->find_most_likely_solutions();
			}

			if (ImGui::BeginListBox("##information-history-listbox", { -1, -1 })) {
				for (auto const& [information, solver] : ui_data.information_history) {
					ImGui::Selectable(information.c_str());
				}
				ImGui::EndListBox();
			}
		}

		ImGui::EndChild();
	}
}

void show_solutions_section(UIData& ui_data) {
	if (ImGui::BeginChild("##solutions")) {
		if (ui_data.solver) {
			for (auto const& [solution, probability] : ui_data.solutions) {
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

void show_new_game_modal(UIData& ui_data) {
	if (ImGui::BeginPopupModal(CSTR(S("UI.NewGame")), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		if (ImGui::InputScalar(CSTR(S("UI.NumberOfPlayers")), ImGuiDataType_U64, &ui_data.new_game_player_count, &u64_one)) {
			ui_data.new_game_player_count = clamp(ui_data.new_game_player_count, Cluedo::Solver::MIN_PLAYER_COUNT, Cluedo::Solver::MAX_PLAYER_COUNT);
			if (ui_data.new_game_players_data.size() != ui_data.new_game_player_count) {
				ui_data.new_game_players_data.resize(ui_data.new_game_player_count);

				auto available_cards = Cluedo::CardUtils::CARD_COUNT - Cluedo::Solver::SOLUTION_CARD_COUNT;
				auto cards_per_player = available_cards / ui_data.new_game_player_count;
				auto remaining_cards = available_cards % ui_data.new_game_player_count;

				for (std::size_t i = 0; i < ui_data.new_game_player_count; ++i) {
					auto& player_data = ui_data.new_game_players_data.at(i);
					player_data.name.clear();
					player_data.n_cards = cards_per_player;
					if (remaining_cards > 0) {
						++player_data.n_cards;
						--remaining_cards;
					}
				}
			}
		}

		ImGui::SeparatorText(CSTR(S("UI.Players")));
		for (size_t i = 0; i < ui_data.new_game_players_data.size(); ++i) {
			ImGui::PushID(i);
			ImGui::InputTextWithHint("##name", fmt::format("{} {}", S("UI.Player"), i + 1).c_str(), &ui_data.new_game_players_data.at(i).name);
			ImGui::SameLine();

			float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
			if (ImGui::ArrowButton("##card_count_decrease", ImGuiDir_Left)) {
				ui_data.new_game_players_data.at(i).n_cards = std::max(ui_data.new_game_players_data.at(i).n_cards - 1, 1uz);
			}
			ImGui::SameLine(0.0f, spacing);
			ImGui::Text("%zu", ui_data.new_game_players_data.at(i).n_cards);
			ImGui::SameLine(0.0f, spacing);
			if (ImGui::ArrowButton("##card_count_increase", ImGuiDir_Right)) {
				ui_data.new_game_players_data.at(i).n_cards = std::max(ui_data.new_game_players_data.at(i).n_cards + 1, 1uz);
			}
			ImGui::SameLine();
			ImGui::TextUnformatted(CSTR(S("UI.Cards")));
			ImGui::PopID();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Button(CSTR(S("UI.Ok")))) {
			auto maybe_solver = Cluedo::Solver::create(ui_data.new_game_players_data);
			if (maybe_solver.is_error()) {
				ui_data.error_message = fmt::format("{}: {}!", S("UI.ErrorWhileCreatingGame"), maybe_solver.release_error());
				ImGui::OpenPopup(CSTR(S("UI.Error")));
			} else {
				ui_data.solver = maybe_solver.release_value();
				ui_data.solutions = ui_data.solver->find_most_likely_solutions();
				ui_data.information_history.clear();
				ImGui::CloseCurrentPopup();
			}
		}

		show_error_modal(ui_data);

		ImGui::SameLine();

		if (ImGui::Button(CSTR(S("UI.Cancel")))) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void show_player_combobox(char const* id, Cluedo::Solver const& solver, size_t& selection) {
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

void show_optional_player_combobox(char const* id, Cluedo::Solver const& solver, std::optional<size_t>& selection) {
	ImGui::PushID(id);
	if (ImGui::BeginCombo("##", selection.has_value() ? solver.player(*selection).name().c_str() : CSTR(S("UI.NoOne")), ImGuiComboFlags_WidthFitPreview)) {
		if (ImGui::Selectable(CSTR(S("UI.NoOne")), !selection.has_value())) {
			selection.reset();
		}

		if (!selection.has_value()) {
			ImGui::SetItemDefaultFocus();
		}

		for (size_t i = 0; i < solver.player_count(); ++i) {
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

void show_card_combobox(char const* id, auto cards_iterator, Cluedo::Card& selection) {
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

void show_optional_card_category_combobox(char const* id, std::optional<Cluedo::CardCategory>& selection) {
	ImGui::PushID(id);
	if (ImGui::BeginCombo("##", selection ? fmt::format("{}", *selection).c_str() : CSTR(S("UI.Unknown")), ImGuiComboFlags_WidthFitPreview)) {
		if (ImGui::Selectable(CSTR(S("UI.Unknown")), !selection.has_value())) {
			selection.reset();
		}
		if (!selection.has_value()) {
			ImGui::SetItemDefaultFocus();
		}

		for (auto card_category : Cluedo::CardUtils::card_categories) {
			bool is_selected = selection == card_category;
			if (ImGui::Selectable(fmt::format("{}", card_category).c_str(), is_selected)) {
				selection = card_category;
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopID();
}

void show_add_information_modal(UIData& ui_data) {
	if (ImGui::BeginPopupModal(CSTR(S("UI.AddInformation")), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		if (ImGui::BeginTabBar("##information-type-tab-bar", ImGuiTabBarFlags_None)) {
			if (ImGui::BeginTabItem(CSTR(S("UI.PlayerHasHasntGotACard")))) {
				ui_data.selected_tab_index = 0;

				show_player_combobox("player-combo", *ui_data.solver, ui_data.first_tab_selected_player);

				ImGui::SameLine();
				ImGui::PushID("has-card-checkbox");
				ImGui::Checkbox(CSTR(S(ui_data.first_tab_is_has_card_checked ? "UI.HasGot" : "UI.HasntGot")), &ui_data.first_tab_is_has_card_checked);
				ImGui::PopID();

				ImGui::SameLine();
				show_card_combobox("card-combo", Cluedo::CardUtils::cards(), ui_data.first_tab_selected_card);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(CSTR(S("UI.PlayerMadeASuggestion")))) {
				ui_data.selected_tab_index = 1;

				show_player_combobox("suggesting-player-combo", *ui_data.solver, ui_data.second_tab_suggestion.suggesting_player_index);

				ImGui::SameLine();
				ImGui::TextUnformatted(CSTR(S("UI.Suggested")));

				ImGui::SameLine();
				show_card_combobox("suspect-combo", Cluedo::CardUtils::cards_per_category(Cluedo::CardCategory::Suspect), ui_data.second_tab_suggestion.suspect);

				ImGui::SameLine();
				show_card_combobox("weapon-combo", Cluedo::CardUtils::cards_per_category(Cluedo::CardCategory::Weapon), ui_data.second_tab_suggestion.weapon);

				ImGui::SameLine();
				show_card_combobox("room-combo", Cluedo::CardUtils::cards_per_category(Cluedo::CardCategory::Room), ui_data.second_tab_suggestion.room);

				show_optional_player_combobox("responding-player-combo", *ui_data.solver, ui_data.second_tab_suggestion.responding_player_index);

				ImGui::SameLine();
				ImGui::TextUnformatted(CSTR(S(ui_data.second_tab_suggestion.responding_player_index ? "UI.RespondedWith" : "UI.Responded")));

				if (ui_data.second_tab_suggestion.responding_player_index) {
					ImGui::SameLine();
					show_optional_card_category_combobox("response-card-combo", ui_data.second_tab_response_card_category);
				}

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Button(CSTR(S("UI.Learn")))) {
			if (ui_data.second_tab_response_card_category) {
				switch (*ui_data.second_tab_response_card_category) {
				case Cluedo::CardCategory::Suspect:
					ui_data.second_tab_suggestion.response_card = ui_data.second_tab_suggestion.suspect;
					break;
				case Cluedo::CardCategory::Weapon:
					ui_data.second_tab_suggestion.response_card = ui_data.second_tab_suggestion.weapon;
					break;
				case Cluedo::CardCategory::Room:
					ui_data.second_tab_suggestion.response_card = ui_data.second_tab_suggestion.room;
					break;
				}
			} else {
				ui_data.second_tab_suggestion.response_card.reset();
			}

			if (ui_data.selected_tab_index == 1 && ui_data.second_tab_suggestion.responding_player_index == ui_data.second_tab_suggestion.suggesting_player_index) {
				ImGui::OpenPopup("Error");
				ui_data.error_message = fmt::format("{}: {}!", S("UI.ErrorWhileLearningNewInformation"), Cluedo::Error::SuggestingPlayerEqualToRespondingPlayer);
			} else {
				auto old_solver = *ui_data.solver;
				std::string information;
				if (ui_data.selected_tab_index == 0) {
					ui_data.solver->learn_player_card_state(ui_data.first_tab_selected_player, ui_data.first_tab_selected_card, ui_data.first_tab_is_has_card_checked);

					auto player_name = ui_data.solver->player(ui_data.first_tab_selected_player).name();
					information = fmt::format("{} {} {}", player_name, S(ui_data.first_tab_is_has_card_checked ? "UI.Has" : "UI.Hasnt"), ui_data.first_tab_selected_card);
				} else if (ui_data.selected_tab_index == 1) {
					ui_data.solver->learn_from_suggestion(ui_data.second_tab_suggestion);

					std::string response;
					if (ui_data.second_tab_suggestion.responding_player_index) {
						auto const& responding_player_name = ui_data.solver->player(*ui_data.second_tab_suggestion.responding_player_index).name();
						if (ui_data.second_tab_suggestion.response_card) {
							response = fmt::format("{} {} {}", responding_player_name, S("UI.RespondedWith"), *ui_data.second_tab_suggestion.response_card);
						} else {
							response = fmt::format("{} {}", responding_player_name, S("UI.Responded"));
						}
					} else {
						response = S("UI.NoOneResponded");
					}

					auto suggestion_player_name = ui_data.solver->player(ui_data.second_tab_suggestion.suggesting_player_index).name();
					auto suspect = ui_data.second_tab_suggestion.suspect;
					auto weapon = ui_data.second_tab_suggestion.weapon;
					auto room = ui_data.second_tab_suggestion.room;
					information = fmt::format("{} {} {}, {}, {} {} {}", suggestion_player_name, S("UI.Suggested"), suspect, weapon, room, S("UI.And"), response);
				}

				if (!ui_data.solver->are_constraints_satisfied()) {
					ImGui::OpenPopup(CSTR(S("UI.Error")));
					ui_data.error_message = fmt::format("{}: {}!", S("UI.ErrorWhileLearningNewInformation"), Cluedo::Error::InvalidInformation);
					ui_data.solver = old_solver;
				} else {
					ui_data.information_history.emplace_back(information, old_solver);
					ui_data.solutions = ui_data.solver->find_most_likely_solutions();
					ImGui::CloseCurrentPopup();
				}
			}
		}

		show_error_modal(ui_data);

		ImGui::EndPopup();
	}
}

void show_player_data_modal(UIData& ui_data) {
	if (ImGui::BeginPopupModal(CSTR(S("UI.PlayerData")), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		for (size_t i = 0; i < ui_data.solver->player_count(); ++i) {
			auto const& player = ui_data.solver->player(i);
			ImGui::TextUnformatted(fmt::format("{} - {} {}", player.name(), player.card_count(), S("UI.Cards")).c_str());
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Button(CSTR(S("UI.Ok")))) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

Result<void, std::string> my_main([[maybe_unused]] std::vector<std::string_view>&& arguments) {
	using namespace std::literals;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		return { SDL_GetError() };
	}

#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100 (WebGL 1.0)
	char const* glsl_version = "#version 100";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
	// GL ES 3.0 + GLSL 300 es (WebGL 2.0)
	char const* glsl_version = "#version 300 es";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
	// GL 3.2 Core + GLSL 150
	char const* glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	// GL 3.0 + GLSL 130
	char const* glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window* window = SDL_CreateWindow("Cluedo Solver", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
	if (window == nullptr) {
		return { SDL_GetError() };
	}

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	if (gl_context == nullptr) {
		return { SDL_GetError() };
	}

	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::GetIO().IniFilename = nullptr;
	ImGui::GetIO().LogFilename = nullptr;
	{
		float base_font_size = 18.0f;
		ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(ibm_plex_sans_regular_compressed_data, ibm_plex_sans_regular_compressed_size, base_font_size);
		ImFontConfig config;
		config.MergeMode = true;
		config.GlyphMinAdvanceX = base_font_size * 2.0f / 3.0f;
		static ImWchar const icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(fa_regular_compressed_data, fa_regular_compressed_size, base_font_size * 2.0 / 3.0f, &config, icon_ranges);
		ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(fa_solid_compressed_data, fa_solid_compressed_size, base_font_size * 2.0 / 3.0f, &config, icon_ranges);
	}

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);

	UIData ui_data;

	while (ui_data.running) {
		ui_data.show_new_game_modal = false;
		ui_data.show_add_information_modal = false;
		ui_data.show_player_data_modal = false;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				ui_data.running = false;

			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
				ui_data.running = false;
		}

		if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
			SDL_Delay(10);
			continue;
		}

		if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_N)) {
			ui_data.initialize_new_game_data();
			ui_data.show_new_game_modal = true;
		}

		if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Enter) && ui_data.solver) {
			ui_data.initialize_add_information_data();
			ui_data.show_add_information_modal = true;
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos({ 0.0f, 0.0f });
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		if (ImGui::Begin("Cluedo Solver", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)) {
			show_menubar(ui_data);

			ImGui::SeparatorText(CSTR(S("UI.InformationHistory")));
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_DelayNormal)) {
				ImGui::SetTooltip("%s", CSTR(S("UI.InformationHistoryTooltipText")));
			}

			show_information_history_section(ui_data);

			ImGui::SeparatorText(CSTR(S("UI.Solutions")));
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_DelayNormal)) {
				ImGui::SetTooltip("%s", CSTR(S("UI.SolutionsTooltipText")));
			}

			show_solutions_section(ui_data);

			ImGui::End();
		}
		ImGui::PopStyleVar();

		if (ui_data.show_new_game_modal) {
			ImGui::OpenPopup(CSTR(S("UI.NewGame")));
		}
		show_new_game_modal(ui_data);

		if (ui_data.show_add_information_modal) {
			ImGui::OpenPopup(CSTR(S("UI.AddInformation")));
		}
		show_add_information_modal(ui_data);

		if (ui_data.show_player_data_modal) {
			ImGui::OpenPopup(CSTR(S("UI.PlayerData")));
		}
		show_player_data_modal(ui_data);

		ImGui::Render();
		glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}

	return {};
}

int main(int argc, char** argv) {
	std::vector<std::string_view> arguments;
	for (int i = 1; i < argc; ++i)
		arguments.emplace_back(argv[i], std::strlen(argv[i]));

	auto maybe_error = my_main(std::move(arguments));
	if (!maybe_error.is_error())
		return EXIT_SUCCESS;

	auto error = maybe_error.release_error();
	fmt::println("[{}] {}", fmt::styled("ERROR", fmt::fg(fmt::color::red)), error);
	return EXIT_FAILURE;
}
