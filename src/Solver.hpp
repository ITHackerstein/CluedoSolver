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

	void learn_player_card_state(std::size_t player_index, Card, bool has_card);
	void learn_player_has_any_of_cards(std::size_t player_index, std::unordered_set<Card> const&);

private:
	explicit Solver(std::vector<Player>&& players)
	  : m_players(std::move(players)) {}

	std::vector<Player> m_players;
};

};
