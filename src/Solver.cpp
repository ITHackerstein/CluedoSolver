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
	auto& player = m_players.at(player_index);

	if (has_card)
		player.add_in_hand_card(card);
	else
		player.add_not_in_hand_card(card);
}

void Solver::learn_player_has_any_of_cards(std::size_t player_index, std::unordered_set<Card> const& card_set) {
	auto& player = m_players.at(player_index);
	player.add_possible_cards(card_set);
}

};
