#include "Solver.hpp"

#include <PCG/pcg_random.hpp>
#include <algorithm>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <numeric>
#include <random>
#include <unordered_map>

namespace Cluedo {

Result<Solver, Error> Solver::create(std::vector<PlayerData> const& players_data) {
	if (players_data.size() < MIN_PLAYER_COUNT || players_data.size() > MAX_PLAYER_COUNT)
		return Error::InvalidNumberOfPlayers;

	std::size_t total_cards = std::accumulate(players_data.begin(), players_data.end(), 3ull, [](std::size_t const& accumulator, PlayerData const& player_data) { return accumulator + player_data.n_cards; });
	if (total_cards != CardUtils::CARD_COUNT)
		return Error::InvalidNumberOfCards;

	std::vector<Player> players;
	for (std::size_t i = 0; i < players_data.size(); ++i) {
		auto name = !players_data.at(i).name.empty() ? players_data.at(i).name : fmt::format("Player {}", i + 1);
		players.emplace_back(name, players_data.at(i).n_cards);
	}
	players.emplace_back("", SOLUTION_CARD_COUNT);

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
		for (auto const& possibility : m_players.at(player_index).m_possibilities) {
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

static void shuffle_cards(std::vector<Card>& cards, pcg64_fast& prng) {
	if (cards.empty())
		return;

	for (std::size_t i = 0; i < cards.size() - 1; ++i) {
		std::size_t j = prng(cards.size() - i) + i;
		std::swap(cards.at(i), cards.at(j));
	}
}

bool Solver::assign_cards_randomly_to_players(std::vector<Card> const& cards) {
	std::size_t next_card_to_assign_index = 0;
	for (std::size_t player_index = 0; player_index < m_players.size() - 1; ++player_index) {
		auto& player = m_players.at(player_index);

		auto cards_to_assign_count = player.n_cards() - player.m_cards_in_hand.size();
		if (cards.size() - next_card_to_assign_index < cards_to_assign_count)
			return false;

		for (std::size_t i = 0; i < cards_to_assign_count; ++i) {
			auto card = cards.at(next_card_to_assign_index++);
			if (player.m_cards_not_in_hand.contains(card))
				return false;

			learn_player_card_state(player_index, card, true, false);
		}
	}

	return true;
}

bool Solver::are_constraints_satisfied() const {
	std::unordered_set<Card> all_player_cards;
	for (auto const& player : m_players) {
		if (player.m_cards_in_hand.size() != player.n_cards())
			return false;

		for (auto card : player.m_cards_in_hand) {
			if (!all_player_cards.insert(card).second)
				return false;
		}

		for (auto possibility : player.m_possibilities) {
			if (std::all_of(possibility.begin(), possibility.end(), [&player](auto const& card) { return !player.m_cards_in_hand.contains(card); }))
				return false;
		}
	}

	return true;
}

std::vector<Solver::SolutionProbabilityPair> Solver::find_most_likely_solutions() const {
	std::unordered_map<CardCategory, std::vector<Card>> possible_solution_cards;
	for (auto const& card : m_players.at(solution_player_index()).m_cards_in_hand)
		possible_solution_cards.insert({ CardUtils::card_category(card), { card } });

	for (auto card_category : CardUtils::card_categories) {
		if (possible_solution_cards.contains(card_category))
			continue;

		possible_solution_cards.insert({ card_category, {} });
		for (auto card : CardUtils::cards_per_category(card_category)) {
			if (m_players.at(solution_player_index()).m_cards_not_in_hand.contains(card))
				continue;

			possible_solution_cards.at(card_category).push_back(card);
		}
	}

	std::unordered_map<Card, std::size_t> iterations_per_card;
	for (auto card : CardUtils::cards())
		iterations_per_card.insert({ card, 0 });

	pcg_extras::seed_seq_from<std::random_device> seed_source;
	pcg64_fast prng(seed_source);

	for (std::size_t iteration = 0; iteration < MAX_ITERATIONS; ++iteration) {
		auto suspect = possible_solution_cards.at(CardCategory::Suspect).at(prng(possible_solution_cards.at(CardCategory::Suspect).size()));
		auto weapon = possible_solution_cards.at(CardCategory::Weapon).at(prng(possible_solution_cards.at(CardCategory::Weapon).size()));
		auto room = possible_solution_cards.at(CardCategory::Room).at(prng(possible_solution_cards.at(CardCategory::Room).size()));

		std::vector<Card> unused_cards;
		for (auto card : CardUtils::cards()) {
			if (card == suspect || card == weapon || card == room)
				continue;

			if (std::any_of(m_players.begin(), m_players.end() - 1, [card](auto const& player) { return player.m_cards_in_hand.contains(card); }))
				continue;

			unused_cards.push_back(card);
		}

		auto solver_copy = *this;
		solver_copy.learn_player_card_state(solver_copy.solution_player_index(), suspect, true, false);
		solver_copy.learn_player_card_state(solver_copy.solution_player_index(), weapon, true, false);
		solver_copy.learn_player_card_state(solver_copy.solution_player_index(), room, true, false);
		solver_copy.infer_new_information();

		shuffle_cards(unused_cards, prng);

		if (solver_copy.assign_cards_randomly_to_players(unused_cards) && solver_copy.are_constraints_satisfied()) {
			++iterations_per_card.at(suspect);
			++iterations_per_card.at(weapon);
			++iterations_per_card.at(room);
		}
	}

	std::vector<SolutionProbabilityPair> solution_probabilities;
	for (auto suspect : CardUtils::cards_per_category(CardCategory::Suspect)) {
		for (auto weapon : CardUtils::cards_per_category(CardCategory::Weapon)) {
			for (auto room : CardUtils::cards_per_category(CardCategory::Room)) {
				auto suspect_probability = static_cast<float>(iterations_per_card.at(suspect)) / MAX_ITERATIONS;
				auto weapon_probability = static_cast<float>(iterations_per_card.at(weapon)) / MAX_ITERATIONS;
				auto room_probability = static_cast<float>(iterations_per_card.at(room)) / MAX_ITERATIONS;
				auto probability = suspect_probability * weapon_probability * room_probability;

				solution_probabilities.emplace_back(std::make_tuple(suspect, weapon, room), probability);
			}
		}
	}

	std::sort(solution_probabilities.begin(), solution_probabilities.end(), [](auto const& a, auto const& b) { return a.second > b.second; });

	return solution_probabilities;
}

};
