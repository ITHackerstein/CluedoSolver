#include "UI.hpp"

#include <fmt/format.h>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <optional>

#include "Strings.hpp"

using namespace std::literals;

namespace ftxui {

class ScrollerBase : public ComponentBase {
public:
	ScrollerBase(Component child) { Add(child); }

private:
	bool Focusable() const override { return true; }

	Element Render() override {
		auto focused = Focused() ? focus : select;
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

	bool OnEvent(Event event) override {
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

	int m_selected = 0;
	int m_size = 0;
	Box m_box;
};

Component Scroller(Component child) { return Make<ScrollerBase>(std::move(child)); }

ButtonOption SmallButtonOption() {
	ButtonOption option;
	option.transform = [](EntryState const& s) {
		auto label_element = text(s.label);
		if (s.focused)
			label_element |= inverted;

		return ftxui::hbox(ftxui::text("["), label_element, ftxui::text("]"));
	};

	return option;
}

}

namespace Cluedo {

namespace UI {

ftxui::Component NumberInput(std::size_t& value, std::optional<std::size_t> minimum = {}, std::optional<std::size_t> maximum = {}, std::optional<std::function<void()>> on_change = {}) {
	class Impl : public ftxui::ComponentBase {
	public:
		Impl(std::size_t& value, std::optional<std::size_t> minimum, std::optional<std::size_t> maximum, std::optional<std::function<void()>> on_change)
		  : m_value(value), m_minimum(minimum), m_maximum(maximum), m_on_change(on_change) {
			m_decrease_button = ftxui::Button(
			  "-", [&]() {
				  if (!m_minimum || m_value > *m_minimum) {
					  --m_value;
					  if (m_on_change)
						  (*m_on_change)();
				  }
			  },
			  ftxui::SmallButtonOption()
			);

			m_increase_button = ftxui::Button(
			  "+", [&]() {
				  if (!m_maximum || m_value < *m_maximum) {
					  ++m_value;
					  if (m_on_change)
						  (*m_on_change)();
				  }
			  },
			  ftxui::SmallButtonOption()
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
		std::size_t& m_value;
		std::optional<std::size_t> m_minimum;
		std::optional<std::size_t> m_maximum;
		std::optional<std::function<void()>> m_on_change;

		ftxui::Component m_decrease_button;
		ftxui::Component m_increase_button;
	};

	return ftxui::Make<Impl>(value, minimum, maximum, on_change);
}

Solver create_solver() {
	auto player_count = Solver::MAX_PLAYER_COUNT;
	std::vector<PlayerData> players_data;
	players_data.resize(Solver::MAX_PLAYER_COUNT);
	std::optional<Error> maybe_error;
	std::optional<Solver> maybe_solver;

	auto reset_player_data = [&]() {
		auto available_cards = CardUtils::CARD_COUNT - Solver::SOLUTION_CARD_COUNT;
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

	auto player_count_number_input = NumberInput(player_count, Solver::MIN_PLAYER_COUNT, Solver::MAX_PLAYER_COUNT, reset_player_data) | ftxui::Renderer([](ftxui::Element inner) { return ftxui::hbox(ftxui::text(fmt::format("{}: ", Strings::the().get_string("UI.NumberOfPlayers"sv))), inner); });

	ftxui::Components player_data_components;
	for (std::size_t i = 0; i < player_count; ++i) {
		auto player_name_input = ftxui::Input(&players_data.at(i).name, fmt::format("{}...", Strings::the().get_string("UI.NamePlaceholder"sv)));
		auto card_number_input = NumberInput(players_data.at(i).n_cards, 1);
		auto player_data_container = ftxui::Container::Horizontal({ player_name_input, card_number_input });
		player_data_container |= ftxui::Renderer([i, player_name_input, card_number_input](ftxui::Element) {
			return ftxui::hbox(
			  ftxui::text(fmt::format("{} {} ", Strings::the().get_string("Solver.Player"sv), i + 1)),
			  player_name_input->Render(),
			  ftxui::separator(),
			  card_number_input->Render(),
			  ftxui::text(fmt::format(" {}", Strings::the().get_string("UI.Cards"sv)))
			);
		});

		player_data_components.push_back(ftxui::Maybe(player_data_container, [i, &player_count]() { return i < player_count; }));
	}

	auto players_data_container = ftxui::Container::Vertical(player_data_components);

	auto submit_button = ftxui::Button(
	  std::string { Strings::the().get_string("UI.StartGame"sv) },
	  [&]() {
		  auto players_data_copy = players_data;
		  players_data_copy.resize(player_count);
		  auto result = Solver::create(players_data_copy);
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
		auto error_text = maybe_error ? ftxui::text(fmt::format("{}: {}", Strings::the().get_string("UI.ErrorPrefix"), *maybe_error)) | ftxui::xflex | ftxui::color(ftxui::Color::Red) : ftxui::emptyElement();

		return ftxui::window(
		  ftxui::text(fmt::format(" {} ", Strings::the().get_string("UI.GameData"sv))),
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

struct ComponentData {
	Solver solver;
	std::vector<std::string> player_names;
	std::vector<std::string> player_names_optional;
	std::vector<std::string> card_categories;
	std::vector<std::string> card_categories_optional;
	std::vector<std::string> cards;
	std::unordered_map<CardCategory, std::vector<std::string>> cards_per_category;

	ComponentData(Solver&& solver_)
	  : solver(std::move(solver_)) {
		player_names_optional.emplace_back(Strings::the().get_string("UI.NoOne"sv));

		for (std::size_t i = 0; i < solver.n_players(); ++i) {
			auto const& name = solver.player(i).name();
			player_names.push_back(name);
			player_names_optional.push_back(name);
		}

		card_categories_optional.emplace_back(Strings::the().get_string("UI.Unknown"sv));
		for (auto category : CardUtils::card_categories) {
			auto card_category_string = fmt::to_string(category);
			card_categories.push_back(card_category_string);
			card_categories_optional.push_back(card_category_string);
			cards_per_category.insert({ category, {} });
			for (auto card : CardUtils::cards_per_category(category)) {
				auto card_string = fmt::to_string(card);
				cards.push_back(card_string);
				cards_per_category.at(category).push_back(card_string);
			}
		}
	}
};

class LearnPlayerCardStateTabBase : public ftxui::ComponentBase {
public:
	LearnPlayerCardStateTabBase(ComponentData& data)
	  : m_player(0), m_has_card(true), m_card(0) {
		m_player_dropdown = ftxui::Dropdown(&data.player_names, &m_player);
		m_has_card_checkbox = ftxui::Checkbox(std::string { Strings::the().get_string("UI.HasGot"sv) }, &m_has_card);
		m_card_dropdown = ftxui::Dropdown(&data.cards, &m_card);

		Add(ftxui::Container::Horizontal({ m_player_dropdown, m_has_card_checkbox, m_card_dropdown }));
	}

	std::size_t player() const { return static_cast<std::size_t>(m_player); }
	bool has_card() const { return m_has_card; }
	Card card() const { return static_cast<Card>(m_card); }

	ftxui::Element Render() override {
		return ftxui::hbox(
		  m_player_dropdown->Render(),
		  m_has_card_checkbox->Render() | ftxui::borderEmpty,
		  m_card_dropdown->Render()
		);
	}

private:
	int m_player;
	bool m_has_card;
	int m_card;

	ftxui::Component m_player_dropdown;
	ftxui::Component m_has_card_checkbox;
	ftxui::Component m_card_dropdown;
};

std::shared_ptr<LearnPlayerCardStateTabBase> LearnPlayerCardStateTab(ComponentData& data) { return ftxui::Make<LearnPlayerCardStateTabBase>(data); }

class LearnFromSuggestionTabBase : public ftxui::ComponentBase {
public:
	LearnFromSuggestionTabBase(ComponentData& data)
	  : m_suggesting_player(0)
	  , m_suspect(0)
	  , m_weapon(0)
	  , m_room(0)
	  , m_responding_player(0)
	  , m_responding_card(0) {
		m_suggesting_player_dropdown = ftxui::Dropdown(&data.player_names, &m_suggesting_player);
		m_suspect_dropdown = ftxui::Dropdown(&data.cards_per_category.at(CardCategory::Suspect), &m_suspect);
		m_weapon_dropdown = ftxui::Dropdown(&data.cards_per_category.at(CardCategory::Weapon), &m_weapon);
		m_room_dropdown = ftxui::Dropdown(&data.cards_per_category.at(CardCategory::Room), &m_room);
		m_responding_player_dropdown = ftxui::Dropdown(&data.player_names_optional, &m_responding_player);
		m_responding_card_dropdown = ftxui::Maybe(ftxui::Dropdown(&data.card_categories_optional, &m_responding_card), [&]() { return m_responding_player != 0; });

		Add(ftxui::Container::Vertical({ ftxui::Container::Horizontal({ m_suggesting_player_dropdown,
		                                                                m_suspect_dropdown,
		                                                                m_weapon_dropdown,
		                                                                m_room_dropdown }),
		                                 ftxui::Container::Horizontal({ m_responding_player_dropdown,
		                                                                m_responding_card_dropdown }) }));
	}

	Result<Solver::Suggestion, Error> suggestion() const {
		Solver::Suggestion suggestion;

		suggestion.suggesting_player_index = static_cast<std::size_t>(m_suggesting_player);
		suggestion.suspect = static_cast<Card>(m_suspect + static_cast<int>(CardCategory::Suspect));
		suggestion.weapon = static_cast<Card>(m_weapon + static_cast<int>(CardCategory::Weapon));
		suggestion.room = static_cast<Card>(m_room + static_cast<int>(CardCategory::Room));

		if (m_responding_player == 0)
			return suggestion;

		suggestion.responding_player_index = static_cast<std::size_t>(m_responding_player - 1);
		if (suggestion.suggesting_player_index == suggestion.responding_player_index)
			return Error::SuggestingPlayerEqualToRespondingPlayer;

		if (m_responding_card != 0) {
			auto card_category = static_cast<CardCategory>(m_responding_card - 1);

			switch (card_category) {
			case CardCategory::Suspect:
				suggestion.response_card = suggestion.suspect;
				break;
			case CardCategory::Weapon:
				suggestion.response_card = suggestion.weapon;
				break;
			case CardCategory::Room:
				suggestion.response_card = suggestion.room;
				break;
			}
		}

		return suggestion;
	}

	ftxui::Element Render() override {
		std::string response_text { Strings::the().get_string(m_responding_player != 0 ? "UI.RespondedWith"sv : "UI.Responded"sv) };

		return ftxui::vbox(
		  ftxui::hbox(
		    m_suggesting_player_dropdown->Render(),
		    ftxui::text(std::string { Strings::the().get_string("UI.Suggested"sv) }) | ftxui::borderEmpty,
		    m_suspect_dropdown->Render(),
		    m_weapon_dropdown->Render(),
		    m_room_dropdown->Render()
		  ),
		  ftxui::hbox(
		    m_responding_player_dropdown->Render(),
		    ftxui::text(response_text) | ftxui::borderEmpty,
		    m_responding_card_dropdown->Render()
		  )
		);
	}

private:
	int m_suggesting_player;
	int m_suspect;
	int m_weapon;
	int m_room;
	int m_responding_player;
	int m_responding_card;

	ftxui::Component m_suggesting_player_dropdown;
	ftxui::Component m_suspect_dropdown;
	ftxui::Component m_weapon_dropdown;
	ftxui::Component m_room_dropdown;
	ftxui::Component m_responding_player_dropdown;
	ftxui::Component m_responding_card_dropdown;
};

std::shared_ptr<LearnFromSuggestionTabBase> LearnFromSuggestionTab(ComponentData& data) { return ftxui::Make<LearnFromSuggestionTabBase>(data); }

class PlayersAndHistoryWindowBase : public ftxui::ComponentBase {
public:
	PlayersAndHistoryWindowBase(ComponentData& data) {
		m_players_list = ftxui::Renderer([&]() {
			ftxui::Elements elements;
			for (std::size_t i = 0; i < data.solver.n_players(); ++i) {
				elements.push_back(
				  ftxui::hbox(
				    ftxui::text(data.solver.player(i).name()) | ftxui::xflex,
				    ftxui::text(" - "),
				    ftxui::text(fmt::format("{} {}", data.solver.player(i).n_cards(), Strings::the().get_string("UI.Cards"sv)))
				  )
				);
			}
			return ftxui::vbox(elements);
		});

		m_undo_button = ftxui::Button(
		  std::string { Strings::the().get_string("UI.Undo"sv) },
		  [&]() {
			  if (m_information_history.empty())
				  return;

			  auto const& [turn, solver] = m_information_history.front();
			  data.solver = solver;
			  m_information_history.erase(m_information_history.begin());
		  },
		  ftxui::SmallButtonOption()
		);

		m_information_history_scroller = ftxui::Scroller(ftxui::Renderer([&]() {
			ftxui::Elements elements;
			for (auto const& [turn, solver] : m_information_history)
				elements.push_back(ftxui::text(turn));
			return ftxui::vbox(elements);
		}));

		Add(ftxui::Container::Horizontal({ m_undo_button, m_information_history_scroller }));
	}

	void add_information(std::string const& turn, Solver const& old_solver) { m_information_history.emplace(m_information_history.begin(), turn, old_solver); }

	ftxui::Element Render() override {
		return ftxui::hbox(
		  ftxui::window(
		    ftxui::text(fmt::format(" {} ", Strings::the().get_string("UI.Players"sv))),
		    m_players_list->Render()
		  ),
		  ftxui::window(
		    ftxui::hbox(
		      ftxui::text(fmt::format(" {} ", Strings::the().get_string("UI.InformationHistory"sv))),
		      m_undo_button->Render(),
		      ftxui::text(" ")
		    ),
		    m_information_history_scroller->Render() | ftxui::xflex | ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, MAX_SCROLLER_HEIGHT)
		  )
		);
	}

private:
	static constexpr int MAX_SCROLLER_HEIGHT = 10;

	std::vector<std::pair<std::string, Solver>> m_information_history;

	ftxui::Component m_players_list;
	ftxui::Component m_undo_button;
	ftxui::Component m_information_history_scroller;
};

std::shared_ptr<PlayersAndHistoryWindowBase> PlayersAndHistoryWindow(ComponentData& data) { return ftxui::Make<PlayersAndHistoryWindowBase>(data); }

class NewInformationWindowBase : public ftxui::ComponentBase {
public:
	NewInformationWindowBase(ComponentData& data, std::function<void(std::string const&, Solver const&)> on_learn)
	  : m_tab_names({ std::string { Strings::the().get_string("UI.PlayerHasHasntGotCard"sv) }, std::string { Strings::the().get_string("UI.PlayerMadeASuggestion"sv) } })
	  , m_selected_tab(0)
	  , m_on_learn(on_learn) {
		m_learn_player_card_state_tab = LearnPlayerCardStateTab(data);
		m_learn_from_suggestion_tab = LearnFromSuggestionTab(data);

		m_tab_toggle = ftxui::Toggle(&m_tab_names, &m_selected_tab);
		m_tab_container = ftxui::Container::Tab({ m_learn_player_card_state_tab, m_learn_from_suggestion_tab }, &m_selected_tab);

		m_learn_button = ftxui::Button(
		  std::string { Strings::the().get_string("UI.Learn"sv) },
		  [&]() {
			  auto old_solver = data.solver;
			  auto result = learn(data.solver);
			  if (result)
				  m_on_learn(*result, old_solver);
		  },
		  ftxui::ButtonOption::Border()
		);

		m_maybe_error_component = ftxui::Maybe(ftxui::Renderer([&]() {
			                                       return ftxui::text(fmt::format("{}: {}", Strings::the().get_string("UI.ErrorPrefix"sv), *m_maybe_error)) | ftxui::color(ftxui::Color::Red);
		                                       }),
		                                       [&]() { return m_maybe_error.has_value(); });

		Add(ftxui::Container::Vertical({ m_tab_toggle, m_tab_container, m_learn_button, m_maybe_error_component }));
	}

	std::optional<std::string> learn(Solver& solver) {
		switch (m_selected_tab) {
		case 0:
			{
				auto player_index = m_learn_player_card_state_tab->player();
				auto card = m_learn_player_card_state_tab->card();
				auto has_card = m_learn_player_card_state_tab->has_card();
				solver.learn_player_card_state(player_index, card, has_card);
				return fmt::format("{} {} {}", solver.player(player_index).name(), Strings::the().get_string(has_card ? "UI.HasGot"sv : "UI.HasntGot"sv), card);
				break;
			}
		case 1:
			{
				auto suggestion_or_error = m_learn_from_suggestion_tab->suggestion();
				if (suggestion_or_error.is_error()) {
					m_maybe_error = suggestion_or_error.release_error();
					return {};
				}

				m_maybe_error.reset();

				auto suggestion = suggestion_or_error.release_value();
				solver.learn_from_suggestion(suggestion);

				std::string response;
				if (!suggestion.responding_player_index)
					response = Strings::the().get_string("UI.NoOneResponded"sv);
				else if (!suggestion.response_card)
					response = fmt::format("{} {}", solver.player(*suggestion.responding_player_index).name(), Strings::the().get_string("UI.Responded"sv));
				else
					response = fmt::format("{} {} {}", solver.player(*suggestion.responding_player_index).name(), Strings::the().get_string("UI.RespondedWith"sv), *suggestion.response_card);

				return fmt::format("{} {} {}, {}, {} - {}", solver.player(suggestion.suggesting_player_index).name(), Strings::the().get_string("UI.Suggested"sv), suggestion.suspect, suggestion.weapon, suggestion.room, response);
				break;
			}
		}

		return {};
	}

	int selected_tab() const { return m_selected_tab; }
	std::shared_ptr<LearnPlayerCardStateTabBase> learn_player_card_state_tab() const { return m_learn_player_card_state_tab; }
	std::shared_ptr<LearnFromSuggestionTabBase> learn_from_suggestion_tab() const { return m_learn_from_suggestion_tab; }

	ftxui::Element Render() override {
		return ftxui::window(
		  ftxui::text(fmt::format(" {} ", Strings::the().get_string("UI.NewInformation"sv))),
		  ftxui::vbox(
		    m_tab_toggle->Render(),
		    ftxui::separator(),
		    m_tab_container->Render(),
		    ftxui::hbox(m_learn_button->Render()),
		    m_maybe_error_component->Render()
		  )
		);
	}

private:
	std::vector<std::string> m_tab_names;
	int m_selected_tab;
	std::function<void(std::string const&, Solver const&)> m_on_learn;
	std::optional<Error> m_maybe_error;

	std::shared_ptr<LearnPlayerCardStateTabBase> m_learn_player_card_state_tab;
	std::shared_ptr<LearnFromSuggestionTabBase> m_learn_from_suggestion_tab;
	ftxui::Component m_tab_toggle;
	ftxui::Component m_tab_container;
	ftxui::Component m_learn_button;
	ftxui::Component m_maybe_error_component;
};

std::shared_ptr<NewInformationWindowBase> NewInformationWindow(ComponentData& data, std::function<void(std::string const&, Solver const&)> on_learn) { return ftxui::Make<NewInformationWindowBase>(data, on_learn); }

class GameWindowBase : public ftxui::ComponentBase {
public:
	GameWindowBase(ComponentData& data, std::function<void()> on_end) {
		m_end_game_button = ftxui::Button(std::string { Strings::the().get_string("UI.EndGame"sv) }, on_end, ftxui::ButtonOption::Border());

		m_players_and_history_window = PlayersAndHistoryWindow(data);
		m_new_information_window = NewInformationWindow(data, [&](std::string const& turn, Solver const& old_solver) { m_players_and_history_window->add_information(turn, old_solver); });

		Add(ftxui::Container::Vertical({ m_players_and_history_window, m_new_information_window, m_end_game_button }));
	}

	ftxui::Element Render() override {
		return ftxui::window(
		  ftxui::text(fmt::format(" {} ", Strings::the().get_string("UI.Game"sv))),
		  ftxui::vbox(
		    m_players_and_history_window->Render(),
		    m_new_information_window->Render(),
		    m_end_game_button->Render()
		  )
		);
	}

private:
	std::shared_ptr<PlayersAndHistoryWindowBase> m_players_and_history_window;
	std::shared_ptr<NewInformationWindowBase> m_new_information_window;
	ftxui::Component m_end_game_button;
};

std::shared_ptr<GameWindowBase> GameWindow(ComponentData& data, std::function<void()> on_end) { return ftxui::Make<GameWindowBase>(data, on_end); }

class SolutionsWindowBase : public ftxui::ComponentBase {
public:
	SolutionsWindowBase(ComponentData& data) {
		m_solutions_scroller = ftxui::Scroller(ftxui::Renderer([&] {
			ftxui::Elements solutions;
			for (auto const& [solution, probability] : m_solutions) {
				auto const& [suspect, weapon, room] = solution;
				solutions.push_back(ftxui::hbox(
				  ftxui::text(fmt::format("{}, {}, {}", suspect, weapon, room)),
				  ftxui::filler(),
				  ftxui::text(fmt::format("{:3.2f} %", probability * 100))
				));
			}
			return ftxui::vbox(solutions) | ftxui::yflex;
		}));

		m_refresh_button = ftxui::Button(
		  std::string { Strings::the().get_string("UI.Refresh"sv) },
		  [&]() {
			  m_solutions = data.solver.find_most_likely_solutions();
		  },
		  ftxui::SmallButtonOption()
		);

		Add(ftxui::Container::Vertical({ m_refresh_button, m_solutions_scroller }));
	}

	ftxui::Element Render() override {
		return ftxui::window(
		  ftxui::hbox(
		    ftxui::text(fmt::format(" {} ", Strings::the().get_string("UI.Solutions"sv))),
		    m_refresh_button->Render(),
		    ftxui::text(" ")
		  ),
		  m_solutions_scroller->Render() | ftxui::yflex
		);
	}

private:
	std::vector<Solver::SolutionProbabilityPair> m_solutions;

	ftxui::Component m_solutions_scroller;
	ftxui::Component m_refresh_button;
};

std::shared_ptr<SolutionsWindowBase> SolutionsWindow(ComponentData& data) { return ftxui::Make<SolutionsWindowBase>(data); }

void main() {
	ComponentData component_data { create_solver() };
	auto screen = ftxui::ScreenInteractive::Fullscreen();

	auto game_window = GameWindow(component_data, [&]() {
		screen.ExitLoopClosure()();
	});

	auto solutions_window = SolutionsWindow(component_data);
	auto container = ftxui::Container::Vertical({ game_window, solutions_window });

	screen.Loop(container);
}

};

};
