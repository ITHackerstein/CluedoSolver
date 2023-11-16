#include "Solver.hpp"

#include <algorithm>
#include <numeric>
#include <unordered_map>

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

void Solver::learn_player_card_state(std::size_t player_index, Card card, bool has_card, bool infer_new_info) {
	if (has_card)
		player(player_index).add_in_hand_card(card);
	else
		player(player_index).add_not_in_hand_card(card);

	if (infer_new_info)
		infer_new_information();
}

void Solver::learn_player_has_any_of_cards(std::size_t player_index, std::unordered_set<Card> const& card_set, bool infer_new_info) {
	player(player_index).add_possible_cards(card_set);

	if (infer_new_info)
		infer_new_information();
}

void Solver::learn_from_suggestion(Suggestion const& suggestion, bool infer_new_info) {
	auto increment_cycling_index = [&](std::size_t index) { return (index + 1) % m_players.size(); };
	auto response_index = suggestion.responding_player_index.value_or(solution_player_index());

	for (auto player_index = increment_cycling_index(suggestion.suggesting_player_index); player_index != response_index; player_index = increment_cycling_index(player_index)) {
		learn_player_card_state(player_index, suggestion.suspect, false, false);
		learn_player_card_state(player_index, suggestion.weapon, false, false);
		learn_player_card_state(player_index, suggestion.room, false, false);
	}

	if (response_index != solution_player_index()) {
		if (suggestion.response_card.has_value())
			learn_player_card_state(response_index, *suggestion.response_card, true, false);
		else
			learn_player_has_any_of_cards(response_index, { suggestion.suspect, suggestion.weapon, suggestion.room }, false);
	}

	if (infer_new_info)
		infer_new_information();
}

void Solver::infer_new_information() {
	// This loops goes over all cards, and checks if all players but one have it.
	// In that case we know the player who owns it.
	for (auto card : CardUtils::cards()) {
		bool card_owned = false;
		std::size_t player_who_dont_own_card_count = 0;
		std::size_t player_index_who_might_have_card = m_players.size();

		for (std::size_t player_index = 0; player_index < m_players.size(); ++player_index) {
			auto card_state = m_players.at(player_index).has_card(card);
			if (!card_state) {
				player_index_who_might_have_card = player_index;
				continue;
			}

			if (!(*card_state)) {
				++player_who_dont_own_card_count;
				continue;
			}

			card_owned = true;
			for (std::size_t other_player_index = 0; other_player_index < m_players.size(); ++other_player_index) {
				if (!m_players.at(other_player_index).has_card(card))
					learn_player_card_state(other_player_index, card, false, false);
			}
		}

		if (!card_owned && player_who_dont_own_card_count == m_players.size() - 1) {
			assert(player_index_who_might_have_card < m_players.size());
			learn_player_card_state(player_index_who_might_have_card, card, true, false);
		}
	}

	// Here we try to infer some more specific information on the solution knowing it has one card of each category.
	for (auto card_category : CardUtils::card_categories) {
		std::optional<Card> solution_card;
		for (auto card : CardUtils::cards_per_category(card_category)) {
			bool is_card_owned = std::any_of(m_players.begin(), m_players.end() - 1, [card](auto const& player) {
				auto card_state = player.has_card(card);
				return card_state && *card_state;
			});

			if (is_card_owned)
				break;

			if (solution_card) {
				solution_card = {};
				break;
			}

			solution_card = card;
		}

		// If we didn't find the solution card or it has already a state we skip to the next category.
		if (!solution_card || m_players.at(solution_player_index()).has_card(*solution_card).has_value())
			continue;

		learn_player_card_state(solution_player_index(), *solution_card, true, false);
	}

	// This loops checks if any players have some possibilities in common.
	// If there are and the number of players is greater than the number of cards in the possibility,
	// then we know that all other players cannot have those cards.
	std::unordered_map<std::unordered_set<Card>, std::unordered_set<std::size_t>> possibilities_to_players_map;
	for (std::size_t player_index = 0; player_index < m_players.size() - 1; ++player_index) {
		for (auto const& possibility : m_players.at(player_index).possibilities()) {
			if (!possibilities_to_players_map.contains(possibility))
				possibilities_to_players_map.insert({ possibility, {} });

			possibilities_to_players_map.at(possibility).insert(player_index);
		}
	}

	for (auto const& [possibility, players] : possibilities_to_players_map) {
		if (players.size() < possibility.size())
			continue;

		for (std::size_t player_index = 0; player_index < m_players.size(); ++player_index) {
			if (players.contains(player_index))
				continue;

			for (auto const& card : possibility)
				learn_player_card_state(player_index, card, false, false);
		}
	}
}

};
