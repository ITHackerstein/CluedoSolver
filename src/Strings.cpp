#include "Strings.hpp"

#include <cassert>
#include <fmt/core.h>
#include <fstream>
#include <ranges>

#include "Card.hpp"

using json = nlohmann::json;
using namespace std::literals;

namespace Cluedo {

std::unique_ptr<Strings> Strings::s_instance = nullptr;
Strings& Strings::the() {
	assert(s_instance != nullptr);

	return *s_instance;
}

static bool has_string(json const& strings, std::string_view key) {
	auto it = strings.find(key);
	return it != strings.end() && it->is_string();
}

Result<void, std::string> Strings::find_error_strings(json const& strings) {
	auto error_it = strings.find("Error"sv);
	if (error_it == strings.end())
		return "language file doesn't contain 'Error' object"s;

	auto const& error_strings = *error_it;
	if (!error_strings.is_object())
		return "'Error' isn't an object"s;

#define _ENUMERATE_ERROR(x)               \
	if (!has_string(error_strings, #x##sv)) \
		return "language file doesn't contain 'Error." #x "'"s;
	_ENUMERATE_ERRORS
#undef _ENUMERATE_ERROR

	return {};
}

Result<void, std::string> Strings::find_card_category_strings(json const& strings) {
	auto card_category_strings_it = strings.find("CardCategory"sv);
	if (card_category_strings_it == strings.end())
		return "language file doesn't contain 'CardCategory' object"s;

	auto const& card_category_strings = *card_category_strings_it;
	if (!card_category_strings.is_object())
		return "'CardCategory' isn't an object"s;

	if (!has_string(card_category_strings, "Suspect"sv))
		return "language file doesn't contain 'CardCategory.Suspect'"s;

	if (!has_string(card_category_strings, "Weapon"sv))
		return "language file doesn't contain 'CardCategory.Weapon'"s;

	if (!has_string(card_category_strings, "Room"sv))
		return "language file doesn't contain 'CardCategory.Room'"s;

	return {};
}

Result<void, std::string> Strings::find_card_strings(json const& strings) {
	auto card_strings_it = strings.find("Card"sv);
	if (card_strings_it == strings.end())
		return "language file doesn't contain 'Card' object"s;

	auto const& card_strings = *card_strings_it;
	if (!card_strings.is_object())
		return "'Card' isn't an object"s;

#define _ENUMERATE_CARD(x)               \
	if (!has_string(card_strings, #x##sv)) \
		return "language file doesn't contain 'Card." #x "'"s;
	_ENUMERATE_CARDS
#undef _ENUMERATE_CARD

	return {};
}

Result<void, std::string> Strings::find_solver_strings(json const& strings) {
	auto solver_strings_it = strings.find("Solver"sv);
	if (solver_strings_it == strings.end())
		return "language file doesn't contain 'Solver' object"s;

	auto const& solver_strings = *solver_strings_it;
	if (!solver_strings.is_object())
		return "'Solver' isn't an object"s;

	if (!has_string(solver_strings, "Player"sv))
		return "language file doesn't contain 'Solver.Player'"s;

	return {};
}

Result<void, std::string> Strings::find_ui_strings(json const& strings) {
	auto ui_strings_it = strings.find("UI"sv);
	if (ui_strings_it == strings.end())
		return "language file doesn't contain 'UI' object"s;

	auto const& ui_strings = *ui_strings_it;
	if (!ui_strings.is_object())
		return "'UI' isn't an object"s;

	if (!has_string(ui_strings, "ErrorPrefix"sv))
		return "language file doesn't contain 'UI.ErrorPrefix'"s;

	if (!has_string(ui_strings, "NumberOfPlayers"sv))
		return "language file doesn't contain 'UI.NumberOfPlayers'"s;

	if (!has_string(ui_strings, "NamePlaceholder"sv))
		return "language file doesn't contain 'UI.NamePlaceholder'"s;

	if (!has_string(ui_strings, "Cards"sv))
		return "language file doesn't contain 'UI.Cards'"s;

	if (!has_string(ui_strings, "StartGame"sv))
		return "language file doesn't contain 'UI.StartGame'"s;

	if (!has_string(ui_strings, "GameData"sv))
		return "language file doesn't contain 'UI.GameData'"s;

	if (!has_string(ui_strings, "NoOne"sv))
		return "language file doesn't contain 'UI.NoOne'"s;

	if (!has_string(ui_strings, "Unknown"sv))
		return "language file doesn't contain 'UI.Unknown'"s;

	if (!has_string(ui_strings, "Suggested"sv))
		return "language file doesn't contain 'UI.Suggested'"s;

	if (!has_string(ui_strings, "RespondedWith"sv))
		return "language file doesn't contain 'UI.RespondedWith'"s;

	if (!has_string(ui_strings, "Responded"sv))
		return "language file doesn't contain 'UI.Responded'"s;

	if (!has_string(ui_strings, "Players"sv))
		return "language file doesn't contain 'UI.Players'"s;

	if (!has_string(ui_strings, "InformationHistory"sv))
		return "language file doesn't contain 'UI.InformationHistory'"s;

	if (!has_string(ui_strings, "PlayerHasHasntGotCard"sv))
		return "language file doesn't contain 'UI.PlayerHasHasntGotCard'"s;

	if (!has_string(ui_strings, "PlayerMadeASuggestion"sv))
		return "language file doesn't contain 'UI.PlayerMadeASuggestion'"s;

	if (!has_string(ui_strings, "Learn"sv))
		return "language file doesn't contain 'UI.Learn'"s;

	if (!has_string(ui_strings, "HasGot"sv))
		return "language file doesn't contain 'UI.HasGot'"s;

	if (!has_string(ui_strings, "HasntGot"sv))
		return "language file doesn't contain 'UI.HasntGot'"s;

	if (!has_string(ui_strings, "NoOneResponded"sv))
		return "language file doesn't contain 'UI.NoOneResponded'"s;

	if (!has_string(ui_strings, "NewInformation"sv))
		return "language file doesn't contain 'UI.NewInformation'"s;

	if (!has_string(ui_strings, "EndGame"sv))
		return "language file doesn't contain 'UI.EndGame'"s;

	if (!has_string(ui_strings, "Game"sv))
		return "language file doesn't contain 'UI.Game'"s;

	if (!has_string(ui_strings, "Refresh"sv))
		return "language file doesn't contain 'UI.Refresh'"s;

	if (!has_string(ui_strings, "Solutions"sv))
		return "language file doesn't contain 'UI.Solutions'"s;

	return {};
}

Result<void, std::string> Strings::load_from_file(std::string_view file_name) {
	json strings;
	try {
		strings = json::parse(std::ifstream(std::string { file_name }));
		if (!strings.is_object())
			return "language file isn't an object"s;
	} catch (json::exception const& exception) {
		return fmt::format("error while parsing language file:\n  {}", exception.what());
	}

	TRY(find_error_strings(strings));
	TRY(find_card_category_strings(strings));
	TRY(find_card_strings(strings));
	TRY(find_solver_strings(strings));
	TRY(find_ui_strings(strings));

	s_instance = std::unique_ptr<Strings>(new Strings(std::move(strings)));
	return {};
}

std::string_view Strings::get_string(std::string_view key) const {
	auto const* obj = &m_strings;
	for (auto part : std::ranges::views::split(key, "."sv)) {
		auto it = obj->find(std::string_view { part });
		if (it == obj->end())
			return key;

		obj = &(*it);
	}

	if (!obj->is_string())
		return key;

	return obj->get<std::string_view>();
}

};
