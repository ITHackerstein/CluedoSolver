#include "Solver.hpp"

#include <numeric>

namespace Cluedo {

Result<Solver, Error> Solver::create(std::vector<PlayerData> const& players_data) {
	if (players_data.size() < MIN_PLAYER_COUNT || players_data.size() > MAX_PLAYER_COUNT)
		return Error::InvalidNumberOfPlayers;

	std::size_t total_cards = std::accumulate(players_data.begin(), players_data.end(), 3ull, [](std::size_t const& accumulator, PlayerData const& player_data) { return accumulator + player_data.n_cards; });
	if (total_cards != CardUtils::CARD_COUNT)
		return Error::InvalidNumberOfCards;

	std::vector<Player> players;
	for (auto const& player_data : players_data)
		players.emplace_back(player_data.name, player_data.n_cards);
	players.emplace_back("", SOLUTION_CARD_COUNT, true);

	return Solver { std::move(players) };
}

void Solver::learn_player_card_state(std::size_t player_index, Card card, bool has_card) {
	if (has_card)
		player(player_index).add_in_hand_card(card);
	else
		player(player_index).add_not_in_hand_card(card);
}

void Solver::learn_player_has_any_of_cards(std::size_t player_index, std::unordered_set<Card> const& card_set) {
	player(player_index).add_possible_cards(card_set);
}

void Solver::learn_from_suggestion(Suggestion const& suggestion) {
	auto increment_cycling_index = [&](std::size_t index) { return (index + 1) % m_players.size(); };
	auto response_index = suggestion.responding_player_index.value_or(solution_player_index());

	for (auto player_index = increment_cycling_index(suggestion.suggesting_player_index); player_index != response_index; player_index = increment_cycling_index(player_index)) {
		learn_player_card_state(player_index, suggestion.suspect, false);
		learn_player_card_state(player_index, suggestion.weapon, false);
		learn_player_card_state(player_index, suggestion.room, false);
	}

	if (response_index != solution_player_index()) {
		if (suggestion.response_card.has_value())
			learn_player_card_state(response_index, *suggestion.response_card, true);
		else
			learn_player_has_any_of_cards(response_index, { suggestion.suspect, suggestion.weapon, suggestion.room });
	}
}

};
