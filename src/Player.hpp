#pragma once

#include "Card.hpp"
#include "CardSet.hpp"

#include <optional>
#include <string>
#include <vector>

namespace Cluedo {

class Solver;

class Player {
	friend Solver;

public:
	explicit Player(std::string const& name, std::size_t n_cards)
	  : m_name(name), m_card_count(n_cards) {}

	std::string const& name() const { return m_name; }
	std::size_t card_count() const { return m_card_count; }

	std::optional<bool> has_card(Card card) const {
		if (m_cards_in_hand.contains(card))
			return true;

		if (m_cards_not_in_hand.contains(card))
			return false;

		return {};
	}

	void add_in_hand_card(Card card) {
		m_cards_in_hand.insert(card);
		simplify_possibilities_with_card(card, true);
		check_if_all_cards_in_hand();
	}

	void add_not_in_hand_card(Card card) {
		m_cards_not_in_hand.insert(card);
		simplify_possibilities_with_card(card, false);
		check_if_all_cards_in_hand();
	}

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
