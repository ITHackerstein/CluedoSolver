#include "Player.hpp"

#include <algorithm>

namespace Cluedo {

static bool is_subset(std::unordered_set<Card> const& a, std::unordered_set<Card> const& b) {
	if (a.size() > b.size())
		return false;

	return std::all_of(a.begin(), a.end(), [&b](auto const& card) { return b.contains(card); });
}

void Player::remove_superfluous_possibilities() {
	for (std::size_t i = 0; i < m_possibilities.size(); ++i) {
		for (std::size_t j = i + 1; j < m_possibilities.size(); ++j) {
			auto const& a = m_possibilities.at(i);
			auto const& b = m_possibilities.at(j);
			if (a == b || is_subset(a, b))
				m_possibilities.at(j).clear();
		}
	}

	for (std::size_t i = m_possibilities.size() - 1; i > 0; --i)
		if (m_possibilities.at(i).empty())
			m_possibilities.erase(m_possibilities.begin() + static_cast<ssize_t>(i - 1));
}

void Player::simplify_possibilities_with_card(Card card, bool has_card) {
	remove_superfluous_possibilities();

	for (std::size_t i = m_possibilities.size(); i > 0; --i) {
		auto it = m_possibilities.begin() + static_cast<ssize_t>(i - 1);
		auto& possibility = m_possibilities.at(i - 1);
		if (!possibility.contains(card))
			continue;

		bool should_erase_possibility = has_card;
		if (!has_card) {
			possibility.erase(card);
			if (possibility.size() == 1) {
				auto resolved_card = possibility.extract(possibility.begin()).value();
				m_cards_in_hand.insert(resolved_card);
				should_erase_possibility = true;
			}
		}

		if (should_erase_possibility)
			m_possibilities.erase(it);
	}
}

};
