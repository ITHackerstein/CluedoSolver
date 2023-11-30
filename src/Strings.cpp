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

#define FIND_STRING(key)                            \
	if (!has_string(card_category_strings, #key##sv)) \
		return "language file doesn't contain 'CardCategory." #key "'"##s;

	FIND_STRING(Suspect)
	FIND_STRING(Weapon)
	FIND_STRING(Room)

#undef FIND_STRING

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

#define FIND_STRING(key)                     \
	if (!has_string(solver_strings, #key##sv)) \
		return "language file doesn't contain 'Solver." #key "'"##s;

	FIND_STRING(Player)

#undef FIND_STRING

	return {};
}

Result<void, std::string> Strings::find_ui_strings(json const& strings) {
	auto ui_strings_it = strings.find("UI"sv);
	if (ui_strings_it == strings.end())
		return "language file doesn't contain 'UI' object"s;

	auto const& ui_strings = *ui_strings_it;
	if (!ui_strings.is_object())
		return "'UI' isn't an object"s;

#define FIND_STRING(key)                     \
	if (!has_string(ui_strings, #key##sv)) \
		return "language file doesn't contain 'UI." #key "'"##s;

	FIND_STRING(ErrorPrefix)
	FIND_STRING(NumberOfPlayers)
	FIND_STRING(NamePlaceholder)
	FIND_STRING(Cards)
	FIND_STRING(StartGame)
	FIND_STRING(GameData)
	FIND_STRING(NoOne)
	FIND_STRING(Unknown)
	FIND_STRING(Suggested)
	FIND_STRING(RespondedWith)
	FIND_STRING(Responded)
	FIND_STRING(Players)
	FIND_STRING(Undo)
	FIND_STRING(InformationHistory)
	FIND_STRING(PlayerHasHasntGotCard)
	FIND_STRING(PlayerMadeASuggestion)
	FIND_STRING(Learn)
	FIND_STRING(HasGot)
	FIND_STRING(HasntGot)
	FIND_STRING(NoOneResponded)
	FIND_STRING(NewInformation)
	FIND_STRING(EndGame)
	FIND_STRING(Game)
	FIND_STRING(Refresh)
	FIND_STRING(Solutions)

#undef FIND_STRING

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
