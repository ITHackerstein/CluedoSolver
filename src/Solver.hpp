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

	Player const& player(std::size_t player_index) const { return m_players.at(player_index); }
	Player& player(std::size_t player_index) { return m_players.at(player_index); }
	std::size_t solution_player_index() const { return m_players.size() - 1; }

	void learn_player_card_state(std::size_t player_index, Card, bool has_card);
	void learn_player_has_any_of_cards(std::size_t player_index, std::unordered_set<Card> const&);

	struct Suggestion {
		std::size_t suggesting_player_index;
		Card suspect;
		Card weapon;
		Card room;
		std::optional<std::size_t> responding_player_index;
		std::optional<Card> response_card;
	};

	void learn_from_suggestion(Suggestion const&);

private:
	explicit Solver(std::vector<Player>&& players)
	  : m_players(std::move(players)) {}

	std::vector<Player> m_players;
};

};
