#pragma once

#include "Error.hpp"
#include "Player.hpp"
#include "Utils/Result.hpp"

namespace Cluedo {

struct PlayerData {
	std::string name;
	std::size_t n_cards;
};

class Solver {
public:
	static constexpr std::size_t MIN_PLAYER_COUNT = 2;
	static constexpr std::size_t MAX_PLAYER_COUNT = 6;
	static constexpr std::size_t SOLUTION_CARD_COUNT = 3;

	static Result<Solver, Error> create(std::vector<PlayerData> const&);

	std::size_t n_players() const { return m_players.size() - 1; }

	Player const& player(std::size_t player_index) const { return m_players.at(player_index); }
	Player& player(std::size_t player_index) { return m_players.at(player_index); }

	void learn_player_card_state(std::size_t player_index, Card, bool has_card, bool infer_new_info = true);
	void learn_player_has_any_of_cards(std::size_t player_index, std::unordered_set<Card> const&, bool infer_new_info = true);

	struct Suggestion {
		std::size_t suggesting_player_index;
		Card suspect;
		Card weapon;
		Card room;
		std::optional<std::size_t> responding_player_index;
		std::optional<Card> response_card;
	};

	void learn_from_suggestion(Suggestion const&, bool infer_new_info = true);

	using SolutionProbabilityPair = std::pair<std::tuple<Card, Card, Card>, float>;
	std::vector<SolutionProbabilityPair> find_most_likely_solutions() const;

private:
	static constexpr std::size_t MAX_ITERATIONS = 200000;

	explicit Solver(std::vector<Player>&& players)
	  : m_players(std::move(players)) {}

	std::size_t solution_player_index() const { return m_players.size() - 1; }

	void infer_new_information();
	bool assign_cards_randomly_to_players(std::vector<Card> const& cards);
	bool are_constraints_satisfied() const;

	std::vector<Player> m_players;
};

};
