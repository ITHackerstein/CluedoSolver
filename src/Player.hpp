#pragma once

#include "Card.hpp"
#include "CardSet.hpp"

#include <optional>
#include <string>
#include <vector>

/// \file Player.hpp
/// \brief The file that contains the definition of the \ref Cluedo::Player class.

namespace Cluedo {

class Solver;

/// \brief The player of a game.
///
/// This class stores the information on a player by using two sets:
/// * the first one will store the cards that we know the player has;
/// * the second one will store the cards that we know the player doesn't have;
/// and a vector that contains _possibilities_, a possibility is a set of cards
/// which tells us that the player must have one of the cards in the set.
///
/// When new information is added to a player then we will try to infer new
/// information on his cards.
class Player {
	friend Solver;

public:
	/// Constructs a player.
	///
	/// \param name The name of the player.
	/// \param card_count The number of cards held by the player.
	explicit Player(std::string const& name, std::size_t card_count)
	  : m_name(name), m_card_count(card_count) {}

	/// Returns the name of the player.
	///
	/// \return The name of the player.
	std::string const& name() const { return m_name; }
	/// Returns the number of cards held by the player.
	///
	/// \return The number of cards held by the player.
	std::size_t card_count() const { return m_card_count; }

	/// Checks if a player has a card.
	///
	/// \return An optional that contains `true` if the player has the card, `false` if he doesn't and nothing if we don't know.
	std::optional<bool> has_card(Card card) const {
		if (m_cards_in_hand.contains(card))
			return true;

		if (m_cards_not_in_hand.contains(card))
			return false;

		return {};
	}

	/// Learns that the player has a card.
	///
	/// \param card The card which the player has.
	void add_in_hand_card(Card card) {
		m_cards_in_hand.insert(card);
		simplify_possibilities_with_card(card, true);
		check_if_all_cards_in_hand();
	}

	/// Learns that the player doesn't have a card.
	///
	/// \param card The card which the player doesn't have.
	void add_not_in_hand_card(Card card) {
		m_cards_not_in_hand.insert(card);
		simplify_possibilities_with_card(card, false);
		check_if_all_cards_in_hand();
	}

	/// Learns that the player has one of the card specified in \a set.
	///
	/// \param set The set of cards of which the player will have one.
	void add_possible_cards(CardSet const& set) {
		m_possibilities.push_back(set);
		remove_superfluous_possibilities();
		check_if_all_cards_in_hand();
	}

private:
	void simplify_possibilities_with_card(Card, bool has_card);
	void remove_superfluous_possibilities();
	void check_if_all_cards_in_hand();

	std::string m_name;
	std::size_t m_card_count;

	CardSet m_cards_in_hand;
	CardSet m_cards_not_in_hand;
	std::vector<CardSet> m_possibilities;
};

};
