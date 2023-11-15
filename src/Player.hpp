#pragma once

#include "Card.hpp"

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace Cluedo {

class Player {
public:
	explicit Player(std::string const& name, std::size_t n_cards)
	  : Player(name, n_cards, false) {}

	explicit Player(std::string const& name, std::size_t n_cards, bool is_solution)
	  : m_name(name), m_n_cards(n_cards), m_is_solution(is_solution) {}

	std::string const& name() const { return m_name; }
	std::size_t n_cards() const { return m_n_cards; }
	bool is_solution() const { return m_is_solution; }

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
	}

	void add_not_in_hand_card(Card card) {
		m_cards_not_in_hand.insert(card);
		simplify_possibilities_with_card(card, false);
	}

	void add_possible_cards(std::unordered_set<Card> const& set) {
		m_possibilities.push_back(set);
		remove_superfluous_possibilities();
	}

private:
	void simplify_possibilities_with_card(Card, bool has_card);
	void remove_superfluous_possibilities();

	std::string m_name;
	std::size_t m_n_cards;
	bool m_is_solution;

	std::unordered_set<Card> m_cards_in_hand;
	std::unordered_set<Card> m_cards_not_in_hand;
	std::vector<std::unordered_set<Card>> m_possibilities;
};

};
