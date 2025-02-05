#pragma once

#include "CardSet.hpp"
#include "Error.hpp"
#include "Player.hpp"
#include "utils/Result.hpp"

/// \file Solver.hpp
/// \brief The file that contains the definition of the \ref Cluedo::Solver class.

namespace Cluedo {

/// \brief A struct that contains the data of a player.
struct PlayerData {
	std::string name;       ///< The name of the player.
	std::size_t card_count; ///< The number of cards held by the player.
};

/// \brief The solver of a Cluedo game.
///
/// This class is the heart of the application. It contains the data of the game
/// and takes information to learn new things about the game. It can also find
/// the most likely solutions for the game.
class Solver {
public:
	static constexpr std::size_t MIN_PLAYER_COUNT = 2;    ///< The minimum number of players that can play a game.
	static constexpr std::size_t MAX_PLAYER_COUNT = 6;    ///< The maximum number of players that can play a game.
	static constexpr std::size_t SOLUTION_CARD_COUNT = 3; ///< The number of cards that make a solution (a suspect, a weapon and a room).

	/// Creates a new \ref Solver object given the data of the players.
	/// \see Cluedo::PlayerData
	///
	/// \param players_data The data of the players.
	///
	/// \return A \ref Result object that contains the \ref Solver object if the
	/// creation was successful or a \ref Cluedo::Error otherwise.
	static Result<Solver, Error> create(std::vector<PlayerData> const& players_data);

	/// Returns the number of players in the game.
	///
	/// \return The number of players in the game.
	std::size_t player_count() const { return m_players.size() - 1; }

	/// Returns the player at the given index.
	///
	/// \param player_index The index of the player.
	///
	/// \return The player at the given index.
	Player const& player(std::size_t player_index) const { return m_players.at(player_index); }
	/// Returns the player at the given index.
	///
	/// \param player_index The index of the player.
	///
	/// \return The player at the given index.
	Player& player(std::size_t player_index) { return m_players.at(player_index); }

	/// Learns that a player has a card or not.
	/// \note This method will infer new information by default.
	///
	/// \param player_index The index of the player.
	/// \param card The card in question.
	/// \param has_card `true` if the player has the card, `false` otherwise.
	/// \param infer_new_info `true` if new information should be inferred, `false` otherwise.
	void learn_player_card_state(std::size_t player_index, Card card, bool has_card, bool infer_new_info = true);

	/// Learns that a player has any of the cards in the given set.
	/// \note This method will infer new information by default.
	///
	/// \param player_index The index of the player.
	/// \param card_set The set of cards in question.
	/// \param infer_new_info `true` if new information should be inferred, `false` otherwise.
	void learn_player_has_any_of_cards(std::size_t player_index, CardSet const& card_set, bool infer_new_info = true);

	/// \brief A struct that contains the data of a suggestion.
	struct Suggestion {
		std::size_t suggesting_player_index;                ///< The index of the player who made the suggestion.
		Card suspect;                                       ///< The suspect suggested.
		Card weapon;                                        ///< The weapon suggested.
		Card room;                                          ///< The room suggested.
		std::optional<std::size_t> responding_player_index; ///< The index of the player who responded, if any.
		std::optional<Card> response_card;                  ///< The card with which the player responded, if known.
	};

	/// Learns from a suggestion.
	/// \note This method will infer new information by default.
	///
	/// \param suggestion The suggestion made in the game.
	/// \param infer_new_info `true` if new information should be inferred, `false` otherwise.
	void learn_from_suggestion(Suggestion const& suggestion, bool infer_new_info = true);

	/// Checks if the constraints of the game are satisfied.
	bool are_constraints_satisfied() const;

	/// \typedef SolutionProbabilityPair
	/// \brief A pair that contains a solution (a suspect, a weapon and a room) and its probability.
	using SolutionProbabilityPair = std::pair<std::tuple<Card, Card, Card>, float>;

	/// Finds the most likely solutions for the game.
	///
	/// \return The list of solutions ordered by their probability.
	std::vector<SolutionProbabilityPair> find_most_likely_solutions() const;

private:
	static constexpr std::size_t MAX_ITERATIONS = 1'000'000;

	explicit Solver(std::vector<Player>&& players)
	  : m_players(std::move(players)) {}

	std::size_t solution_player_index() const { return m_players.size() - 1; }

	void infer_new_information();
	bool assign_cards_to_players(std::vector<Card> const& cards);
	bool are_constraints_satisfied_for_solution_search() const;

	std::vector<Player> m_players;
};

};
