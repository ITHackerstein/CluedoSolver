#include "UI.hpp"

#include <fmt/format.h>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <optional>

namespace ftxui {

Element ScrollerBase::Render() {
	auto focused = Focused() ? focus : ftxui::select;
	auto style = Focused() ? inverted : nothing;

	Element background = ComponentBase::Render();
	background->ComputeRequirement();
	m_size = background->requirement().min_y;

	if (m_size == 0)
		return dbox({ std::move(background) }) | vscroll_indicator | yframe | yflex | reflect(m_box);

	return dbox({
	         std::move(background),
	         vbox({
	           text(L"") | size(HEIGHT, EQUAL, m_selected),
	           text(L"") | style | focused,
	         }),
	       })
	  | vscroll_indicator | yframe | yflex | reflect(m_box);
}

bool ScrollerBase::OnEvent(Event event) {
	if (event.is_mouse() && m_box.Contain(event.mouse().x, event.mouse().y))
		TakeFocus();

	int selected_old = m_selected;
	if (event == Event::ArrowUp || event == Event::Character('k') || (event.is_mouse() && event.mouse().button == Mouse::WheelUp)) {
		m_selected--;
	}
	if ((event == Event::ArrowDown || event == Event::Character('j') || (event.is_mouse() && event.mouse().button == Mouse::WheelDown))) {
		m_selected++;
	}
	if (event == Event::PageDown)
		m_selected += m_box.y_max - m_box.y_min;
	if (event == Event::PageUp)
		m_selected -= m_box.y_max - m_box.y_min;
	if (event == Event::Home)
		m_selected = 0;
	if (event == Event::End)
		m_selected = m_size;

	m_selected = std::max(0, std::min(m_size - 1, m_selected));
	return selected_old != m_selected;
}

Component Scroller(Component child) {
	return Make<ScrollerBase>(std::move(child));
}

}

namespace Cluedo {

namespace UI {

template<typename ValueType>
ftxui::Component NumberInput(ValueType& value, std::optional<ValueType> minimum, std::optional<ValueType> maximum, std::optional<std::function<void()>> on_change) {
	class Impl : public ftxui::ComponentBase {
	public:
		Impl(ValueType& value, std::optional<ValueType> minimum, std::optional<ValueType> maximum, std::optional<std::function<void()>> on_change)
		  : m_value(value), m_minimum(minimum), m_maximum(maximum), m_on_change(on_change) {
			m_decrease_button = ftxui::Button(
			  "-", [&]() {
				  if (!m_minimum || m_value > *m_minimum) {
					  --m_value;
					  if (m_on_change)
						  (*m_on_change)();
				  }
			  },
			  ftxui::ButtonOption::Ascii()
			);

			m_increase_button = ftxui::Button(
			  "+", [&]() {
				  if (!m_maximum || m_value < *m_maximum) {
					  ++m_value;
					  if (m_on_change)
						  (*m_on_change)();
				  }
			  },
			  ftxui::ButtonOption::Ascii()
			);

			Add(ftxui::Container::Horizontal({ m_decrease_button, m_increase_button }));
		}

		ftxui::Element Render() override {
			return ftxui::hbox(
			  m_decrease_button->Render(),
			  ftxui::text(fmt::format(" {} ", m_value)),
			  m_increase_button->Render()
			);
		}

	private:
		ValueType& m_value;
		std::optional<ValueType> m_minimum;
		std::optional<ValueType> m_maximum;
		std::optional<std::function<void()>> m_on_change;

		ftxui::Component m_decrease_button;
		ftxui::Component m_increase_button;
	};

	return ftxui::Make<Impl>(value, minimum, maximum, on_change);
}

Solver create_solver() {
	std::size_t player_count = Cluedo::Solver::MAX_PLAYER_COUNT;
	std::vector<Cluedo::PlayerData> players_data;
	players_data.resize(Cluedo::Solver::MAX_PLAYER_COUNT);
	std::optional<Cluedo::Error> maybe_error;
	std::optional<Cluedo::Solver> maybe_solver;

	auto reset_player_data = [&]() {
		auto available_cards = Cluedo::CardUtils::CARD_COUNT - Cluedo::Solver::SOLUTION_CARD_COUNT;
		auto cards_per_player = available_cards / player_count;
		auto remaining_cards = available_cards % player_count;

		for (std::size_t i = 0; i < player_count; ++i) {
			players_data.at(i).name.clear();
			players_data.at(i).n_cards = cards_per_player;
			if (remaining_cards > 0) {
				++players_data.at(i).n_cards;
				--remaining_cards;
			}
		}
	};

	reset_player_data();

	auto screen = ftxui::ScreenInteractive::Fullscreen();

	auto player_count_number_input = NumberInput(player_count, std::make_optional(Cluedo::Solver::MIN_PLAYER_COUNT), std::make_optional(Cluedo::Solver::MAX_PLAYER_COUNT), reset_player_data) | ftxui::Renderer([](ftxui::Element inner) { return ftxui::hbox(ftxui::text("Number of players: "), inner); });

	ftxui::Components player_data_components;
	for (std::size_t i = 0; i < player_count; ++i) {
		auto player_name_input = ftxui::Input(&players_data.at(i).name, "Name...");
		auto card_number_input = NumberInput(players_data.at(i).n_cards, std::make_optional(static_cast<std::size_t>(1)));
		auto player_data_container = ftxui::Container::Horizontal({ player_name_input, card_number_input }) | ftxui::Renderer([i, player_name_input, card_number_input](ftxui::Element) {
			                             return ftxui::hbox(
			                               ftxui::text(fmt::format("Player {} ", i + 1)),
			                               player_name_input->Render(),
			                               ftxui::separator(),
			                               card_number_input->Render(),
			                               ftxui::text(" cards")
			                             );
		                             });

		player_data_components.push_back(ftxui::Maybe(player_data_container, [i, &player_count]() { return i < player_count; }));
	}

	auto players_data_container = ftxui::Container::Vertical(player_data_components);

	auto submit_button = ftxui::Button(
	  "Start game", [&]() {
		  auto result = Cluedo::Solver::create(players_data);
		  if (result.is_error()) {
			  maybe_error = result.release_error();
			  return;
		  }

		  maybe_solver = result.release_value();
		  screen.ExitLoopClosure()();
	  },
	  ftxui::ButtonOption::Border()
	);

	auto container = ftxui::Container::Vertical({ player_count_number_input,
	                                              players_data_container,
	                                              submit_button });

	auto renderer = ftxui::Renderer(container, [&]() {
		auto error_text = maybe_error ? ftxui::text(fmt::format("ERROR: {}", *maybe_error)) | ftxui::xflex | ftxui::color(ftxui::Color::Red) : ftxui::emptyElement();

		return ftxui::window(
		  ftxui::text(" Game data "),
		  ftxui::vbox(
		    player_count_number_input->Render(),
		    ftxui::separator(),
		    players_data_container->Render(),
		    ftxui::separator(),
		    submit_button->Render(),
		    error_text
		  )
		);
	});

	screen.Loop(renderer);

	return *maybe_solver;
}

void main() {
	auto solver = Cluedo::UI::create_solver();

	std::vector<std::string> player_names;
	for (std::size_t i = 0; i < solver.n_players(); ++i)
		player_names.push_back(solver.player(i).name());

	std::vector<std::string> cards;
	std::unordered_map<Cluedo::CardCategory, std::vector<std::string>> cards_per_category;

	for (auto category : Cluedo::CardUtils::card_categories) {
		cards_per_category.insert({ category, {} });
		for (auto card : Cluedo::CardUtils::cards_per_category(category)) {
			auto s = fmt::to_string(card);
			cards.push_back(s);
			cards_per_category.at(category).push_back(s);
		}
	}

	std::vector<std::string> information_history;
	std::vector<Cluedo::Solver::SolutionProbabilityPair> most_likely_solutions;

	int player_card_state_player = 0;
	auto player_card_state_player_dropdown = ftxui::Dropdown(&player_names, &player_card_state_player);

	int player_card_state_card = 0;
	auto player_card_state_card_dropdown = ftxui::Dropdown(&cards, &player_card_state_card);

	bool player_card_state_has_card = true;
	auto player_card_state_has_card_checkbox = ftxui::Checkbox("has card", &player_card_state_has_card);

	auto player_card_state_container = ftxui::Container::Vertical({ player_card_state_player_dropdown,
	                                                                player_card_state_has_card_checkbox,
	                                                                player_card_state_card_dropdown });

	auto player_card_state_container_renderer = ftxui::Renderer(player_card_state_container, [&]() {
		return ftxui::hbox(
		  player_card_state_player_dropdown->Render(),
		  player_card_state_has_card_checkbox->Render() | ftxui::center,
		  player_card_state_card_dropdown->Render()
		);
	});

	int player_suggestion_suggesting_player = 0;
	auto player_suggestion_suggesting_player_dropdown = ftxui::Dropdown(&player_names, &player_suggestion_suggesting_player);

	int player_suggestion_suspect = 0;
	auto player_suggestion_suspect_dropdown = ftxui::Dropdown(&cards_per_category.at(Cluedo::CardCategory::Suspect), &player_suggestion_suspect);

	int player_suggestion_weapon = 0;
	auto player_suggestion_weapon_dropdown = ftxui::Dropdown(&cards_per_category.at(Cluedo::CardCategory::Weapon), &player_suggestion_weapon);

	int player_suggestion_room = 0;
	auto player_suggestion_room_dropdown = ftxui::Dropdown(&cards_per_category.at(Cluedo::CardCategory::Room), &player_suggestion_room);

	std::vector<std::string> player_suggestion_responding_player_dropdown_entries = player_names;
	player_suggestion_responding_player_dropdown_entries.insert(player_suggestion_responding_player_dropdown_entries.begin(), "No one");

	int player_suggestion_responding_player = 0;
	auto player_suggestion_responding_player_dropdown = ftxui::Dropdown(&player_suggestion_responding_player_dropdown_entries, &player_suggestion_responding_player);

	auto player_response_text = ftxui::Renderer([&]() {
		return (player_suggestion_responding_player != 0 ? ftxui::text("responded with") : ftxui::text("responded")) | ftxui::center;
	});

	std::vector<std::string> player_suggestion_responding_card_dropdown_entries { "Unknown" };
	for (auto category : Cluedo::CardUtils::card_categories)
		player_suggestion_responding_card_dropdown_entries.push_back(fmt::to_string(category));

	int player_suggestion_responding_card = 0;
	auto player_suggestion_responding_card_dropdown = ftxui::Maybe(ftxui::Dropdown(&player_suggestion_responding_card_dropdown_entries, &player_suggestion_responding_card), [&]() {
		return player_suggestion_responding_player != 0;
	});

	auto player_suggestion_container = ftxui::Container::Vertical({ ftxui::Container::Horizontal({ player_suggestion_suggesting_player_dropdown,
	                                                                                               player_suggestion_suspect_dropdown,
	                                                                                               player_suggestion_weapon_dropdown,
	                                                                                               player_suggestion_room_dropdown }),
	                                                                ftxui::Container::Horizontal({ player_suggestion_responding_player_dropdown,
	                                                                                               player_suggestion_responding_card_dropdown }) });

	auto player_suggestion_container_renderer = ftxui::Renderer(player_suggestion_container, [&]() {
		return ftxui::vbox(
		  ftxui::hbox(
		    player_suggestion_suggesting_player_dropdown->Render(),
		    ftxui::text("suggested") | ftxui::center,
		    player_suggestion_suspect_dropdown->Render(),
		    player_suggestion_weapon_dropdown->Render(),
		    player_suggestion_room_dropdown->Render()
		  ),
		  ftxui::hbox(
		    player_suggestion_responding_player_dropdown->Render(),
		    player_response_text->Render(),
		    player_suggestion_responding_card_dropdown->Render()
		  )
		);
	});

	std::vector<std::string> new_information_tab_names { "Player has/hasn't got card", "Player made a suggestion" };
	int new_information_selected_tab = 0;
	auto new_information_tab_menu = ftxui::Menu(&new_information_tab_names, &new_information_selected_tab);

	auto new_information_tab_container = ftxui::Container::Tab({ player_card_state_container_renderer, player_suggestion_container_renderer }, &new_information_selected_tab);

	auto learn_button = ftxui::Button("Learn", [&]() {
		if (new_information_selected_tab == 0) {
			auto player_index = static_cast<std::size_t>(player_card_state_player);
			auto card = static_cast<Cluedo::Card>(player_card_state_card);
			solver.learn_player_card_state(player_index, card, player_card_state_has_card);
			information_history.push_back(fmt::format("{} {} {}", solver.player(player_index).name(), player_card_state_has_card ? "has got" : "hasn't got", card));
		} else {
			Cluedo::Solver::Suggestion suggestion;

			suggestion.suggesting_player_index = static_cast<std::size_t>(player_suggestion_suggesting_player);
			if (player_suggestion_responding_player != 0)
				suggestion.responding_player_index = static_cast<std::size_t>(player_suggestion_responding_player - 1);
			suggestion.suspect = static_cast<Cluedo::Card>(static_cast<int>(Cluedo::CardCategory::Suspect) + player_suggestion_suspect);
			suggestion.weapon = static_cast<Cluedo::Card>(static_cast<int>(Cluedo::CardCategory::Weapon) + player_suggestion_weapon);
			suggestion.room = static_cast<Cluedo::Card>(static_cast<int>(Cluedo::CardCategory::Room) + player_suggestion_room);
			switch (player_suggestion_responding_card) {
			case 1:
				suggestion.response_card = suggestion.suspect;
				break;
			case 2:
				suggestion.response_card = suggestion.weapon;
				break;
			case 3:
				suggestion.response_card = suggestion.room;
				break;
			default:
				break;
			}
			solver.learn_from_suggestion(suggestion);

			std::string response;
			if (!suggestion.responding_player_index)
				response = "No one responded!";
			else if (!suggestion.response_card)
				response = fmt::format("{} responded", solver.player(*suggestion.responding_player_index).name());
			else
				response = fmt::format("{} responded with {}", solver.player(*suggestion.responding_player_index).name(), *suggestion.response_card);

			information_history.insert(information_history.begin(), fmt::format("{} suggested {}, {}, {} - {}", solver.player(suggestion.suggesting_player_index).name(), suggestion.suspect, suggestion.weapon, suggestion.room, response));
		}
	});

	auto new_information_container = ftxui::Container::Horizontal({ new_information_tab_menu, new_information_tab_container, learn_button });

	auto new_information_container_renderer = ftxui::Renderer(new_information_container, [&]() {
		return ftxui::hbox(
		  new_information_tab_menu->Render(),
		  ftxui::separator(),
		  new_information_tab_container->Render(),
		  ftxui::separator(),
		  ftxui::vbox(learn_button->Render())
		);
	});

	auto screen = ftxui::ScreenInteractive::Fullscreen();

	auto end_game_button = ftxui::Button("End game", [&]() {
		screen.ExitLoopClosure()();
	});

	auto information_history_scroller = ftxui::Scroller(ftxui::Renderer([&] {
		ftxui::Elements turns;
		for (auto const& turn : information_history) {
			turns.push_back(ftxui::text(turn));
		}
		return ftxui::vbox(turns);
	}));

	auto game_container = ftxui::Container::Vertical({ information_history_scroller, new_information_container_renderer, end_game_button });

	auto game_container_renderer = ftxui::Renderer(game_container, [&]() {
		ftxui::Elements players_data_container;
		for (std::size_t i = 0; i < solver.n_players(); ++i) {
			players_data_container.push_back(ftxui::text(fmt::format("{} - {} cards", solver.player(i).name(), solver.player(i).n_cards())));
		}

		return ftxui::vbox(
		  ftxui::hbox(
		    ftxui::window(
		      ftxui::text(" Players "),
		      ftxui::vbox(players_data_container)
		    ),
		    ftxui::window(
		      ftxui::text(" Information history "),
		      information_history_scroller->Render() | ftxui::xflex | ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 10)
		    )
		  ),
		  ftxui::window(
		    ftxui::text(" New information "),
		    new_information_container_renderer->Render()
		  ),
		  end_game_button->Render()
		);
	});

	auto refresh_solutions_button = ftxui::Button(
	  "Refresh", [&]() {
		  most_likely_solutions = solver.find_most_likely_solutions();
	  },
	  ftxui::ButtonOption::Ascii()
	);

	auto solutions_scroller = ftxui::Scroller(ftxui::Renderer([&] {
		ftxui::Elements solutions;
		for (auto const& [solution, probability] : most_likely_solutions) {
			auto const& [suspect, weapon, room] = solution;
			solutions.push_back(ftxui::hbox(
			  ftxui::text(fmt::format("{} {} {}", suspect, weapon, room)),
			  ftxui::filler(),
			  ftxui::text(fmt::format("{:3.2f} %", probability * 100))
			));
		}
		return ftxui::vbox(solutions) | ftxui::yflex;
	}));

	auto container = ftxui::Container::Vertical({ game_container_renderer, refresh_solutions_button, solutions_scroller });

	auto renderer = ftxui::Renderer(container, [&]() {
		return ftxui::vbox(
		  ftxui::window(
		    ftxui::text(" Game "),
		    game_container_renderer->Render()
		  ),
		  ftxui::window(
		    ftxui::hbox(ftxui::text(" Solutions "), refresh_solutions_button->Render(), ftxui::text(" ")),
		    solutions_scroller->Render() | ftxui::yflex
		  )
		);
	});

	screen.Loop(renderer);
}

};

};
