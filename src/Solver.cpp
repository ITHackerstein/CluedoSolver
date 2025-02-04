#include "Solver.hpp"

#include <algorithm>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <numeric>
#include <pcg_random.hpp>
#include <random>
#include <unordered_map>
#include <unordered_set>

#include "LanguageStrings.hpp"

namespace Cluedo {

Result<Solver, Error> Solver::create(std::vector<PlayerData> const& players_data) {
	if (players_data.size() < MIN_PLAYER_COUNT || players_data.size() > MAX_PLAYER_COUNT)
		return Error::InvalidNumberOfPlayers;

	std::size_t total_cards = std::accumulate(players_data.begin(), players_data.end(), SOLUTION_CARD_COUNT, [](std::size_t const& accumulator, PlayerData const& player_data) { return accumulator + player_data.n_cards; });
	if (total_cards != CardUtils::CARD_COUNT)
		return Error::InvalidNumberOfCards;

	std::vector<Player> players;
	for (std::size_t i = 0; i < players_data.size(); ++i) {
		auto name = !players_data.at(i).name.empty() ? players_data.at(i).name : fmt::format("{} {}", Cluedo::LanguageStrings::the().get_string("Solver.Player"), i + 1);
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

	if (player_index == solution_player_index() && has_card) {
		for (auto other_card : CardUtils::cards_per_category(CardUtils::card_category(card))) {
			if (other_card != card)
				learn_player_card_state(player_index, other_card, false, false);
		}
	}

	if (infer_new_info)
		infer_new_information();
}

void Solver::learn_player_has_any_of_cards(std::size_t player_index, CardSet const& card_set, bool infer_new_info) {
	player(player_index).add_possible_cards(card_set);

	if (infer_new_info)
		infer_new_information();
}

void Solver::learn_from_suggestion(Suggestion const& suggestion, bool infer_new_info) {
	auto response_index = suggestion.responding_player_index.value_or(solution_player_index());

	auto increment_cycling_index = [&](std::size_t index) { return (index + 1) % (m_players.size() - 1); };
	for (auto player_index = increment_cycling_index(suggestion.suggesting_player_index); player_index != suggestion.suggesting_player_index && player_index != response_index; player_index = increment_cycling_index(player_index)) {
		learn_player_card_state(player_index, suggestion.suspect, false, false);
		learn_player_card_state(player_index, suggestion.weapon, false, false);
		learn_player_card_state(player_index, suggestion.room, false, false);
	}

	if (response_index == solution_player_index()) {
		learn_player_card_state(response_index, suggestion.suspect, true, false);
		learn_player_card_state(response_index, suggestion.weapon, true, false);
		learn_player_card_state(response_index, suggestion.room, true, false);
	} else {
		if (suggestion.response_card.has_value())
			learn_player_card_state(response_index, *suggestion.response_card, true, false);
		else
			learn_player_has_any_of_cards(response_index, { suggestion.suspect, suggestion.weapon, suggestion.room }, false);
	}

	if (infer_new_info)
		infer_new_information();
}

bool Solver::are_constraints_satisfied() const {
	for (auto const& player : m_players) {
		if (!CardSet::intersection(player.m_cards_in_hand, player.m_cards_not_in_hand).empty())
			return false;
	}

	return true;
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
			break;
		}

		if (!card_owned && player_who_dont_own_card_count == m_players.size() - 1) {
			assert(player_index_who_might_have_card < m_players.size());
			learn_player_card_state(player_index_who_might_have_card, card, true, false);
		}
	}

	// Here we try to infer some more specific information on the solution knowing it has one card of each category.
	for (auto card_category : CardUtils::card_categories) {
		bool solution_has_category = false;
		std::optional<Card> solution_card;

		for (auto card : CardUtils::cards_per_category(card_category)) {
			bool is_owned_by_solution = player(solution_player_index()).m_cards_in_hand.contains(card);
			if (is_owned_by_solution) {
				solution_has_category = true;
				break;
			}

			bool is_card_owned = std::any_of(m_players.begin(), m_players.end() - 1, [card](auto const& player) { return player.m_cards_in_hand.contains(card); });

			if (is_card_owned)
				continue;

			if (solution_card) {
				solution_card.reset();
				break;
			} else {
				solution_card = card;
			}
		}

		// If the solution doesn't have a card of this category and we found the solution card
		if (!solution_has_category && solution_card)
			learn_player_card_state(solution_player_index(), *solution_card, true, false);
	}

	// This loops checks if any players have some possibilities in common.
	// If there are and the number of players is greater than the number of cards in the possibility,
	// then we know that all other players cannot have those cards.
	std::unordered_map<CardSet, std::unordered_set<std::size_t>> possibilities_to_players_map;
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

bool Solver::assign_cards_to_players(std::vector<Card> const& cards) {
	std::size_t next_card_to_assign_index = 0;
	for (std::size_t player_index = 0; player_index < m_players.size() - 1; ++player_index) {
		auto& p = player(player_index);

		auto cards_to_assign_count = p.card_count() - p.m_cards_in_hand.size();
		if (cards.size() - next_card_to_assign_index < cards_to_assign_count)
			return false;

		for (std::size_t i = 0; i < cards_to_assign_count; ++i) {
			auto card = cards.at(next_card_to_assign_index++);
			if (p.m_cards_not_in_hand.contains(card))
				return false;

			p.m_cards_in_hand.insert(card);
		}
	}

	return true;
}

bool Solver::are_constraints_satisfied_for_solution_search() const {
	CardSet all_player_cards;
	for (auto const& player : m_players) {
		if (player.m_cards_in_hand.size() != player.card_count())
			return false;

		if (!CardSet::intersection(all_player_cards, player.m_cards_in_hand).empty())
			return false;

		all_player_cards.set_union(player.m_cards_in_hand);

		for (auto const& possibility : player.m_possibilities) {
			if (CardSet::intersection(possibility, player.m_cards_in_hand).empty())
				return false;
		}
	}

	return true;
}

std::vector<Solver::SolutionProbabilityPair> Solver::find_most_likely_solutions() const {
	std::unordered_map<CardCategory, CardSet> possible_solution_cards;
	for (auto const& card : player(solution_player_index()).m_cards_in_hand)
		possible_solution_cards.insert({ CardUtils::card_category(card), { card } });

	for (auto card_category : CardUtils::card_categories) {
		if (possible_solution_cards.contains(card_category))
			continue;

		possible_solution_cards.insert({ card_category, {} });
		for (auto card : CardUtils::cards_per_category(card_category)) {
			if (player(solution_player_index()).m_cards_not_in_hand.contains(card))
				continue;

			possible_solution_cards.at(card_category).insert(card);
		}
	}

	auto solution_count = possible_solution_cards.at(CardCategory::Suspect).size() * possible_solution_cards.at(CardCategory::Weapon).size() * possible_solution_cards.at(CardCategory::Room).size();
	auto max_iterations_per_solution = MAX_ITERATIONS / solution_count;

	pcg_extras::seed_seq_from<std::random_device> seed_source;
	pcg64_fast prng(seed_source);

	std::vector<SolutionProbabilityPair> solution_probabilities;

	for (auto suspect : CardUtils::cards_per_category(CardCategory::Suspect)) {
		for (auto weapon : CardUtils::cards_per_category(CardCategory::Weapon)) {
			for (auto room : CardUtils::cards_per_category(CardCategory::Room)) {
				if (!possible_solution_cards.at(CardCategory::Suspect).contains(suspect) || !possible_solution_cards.at(CardCategory::Weapon).contains(weapon) || !possible_solution_cards.at(CardCategory::Room).contains(room))
					continue;

				auto solver_first_copy = *this;

				solver_first_copy.learn_player_card_state(solver_first_copy.solution_player_index(), suspect, true, false);
				solver_first_copy.learn_player_card_state(solver_first_copy.solution_player_index(), weapon, true, false);
				solver_first_copy.learn_player_card_state(solver_first_copy.solution_player_index(), room, true, false);

				std::vector<Card> unused_cards;
				for (auto card : CardUtils::cards()) {
					if (std::any_of(solver_first_copy.m_players.begin(), solver_first_copy.m_players.end(), [card](auto const& player) { return player.m_cards_in_hand.contains(card); }))
						continue;

					unused_cards.push_back(card);
				}

				solver_first_copy.infer_new_information();

				std::size_t valid_iterations = 0;
				for (std::size_t iteration = 0; iteration < max_iterations_per_solution; ++iteration) {
					auto solver_second_copy = solver_first_copy;

					shuffle_cards(unused_cards, prng);

					if (solver_second_copy.assign_cards_to_players(unused_cards) && solver_second_copy.are_constraints_satisfied_for_solution_search())
						++valid_iterations;
				}

				solution_probabilities.emplace_back(std::make_tuple(suspect, weapon, room), valid_iterations);
			}
		}
	}

	auto total_iterations = std::accumulate(solution_probabilities.begin(), solution_probabilities.end(), 0.0f, [](auto const& accumulator, auto const& pair) { return accumulator + pair.second; });

	for (auto& pair : solution_probabilities)
		pair.second /= total_iterations;

	std::sort(solution_probabilities.begin(), solution_probabilities.end(), [](auto const& a, auto const& b) { return a.second > b.second; });

	return solution_probabilities;
}

};
