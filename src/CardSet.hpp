#pragma once

#include <bitset>
#include <initializer_list>

#include "Card.hpp"

namespace Cluedo {

class CardSet {
public:
	friend std::hash<CardSet>;

	class iterator {
	public:
		constexpr iterator(std::bitset<CardUtils::CARD_COUNT> const& set_ref)
		  : m_set_ref(set_ref) {
			advance_to_next_index();
		}

		constexpr iterator(std::bitset<CardUtils::CARD_COUNT> const& set_ref, std::size_t index)
		  : m_set_ref(set_ref), m_index(index) {
			advance_to_next_index();
		}

		constexpr void advance_to_next_index() {
			while (m_index < m_set_ref.size() && !m_set_ref[m_index])
				++m_index;
		}

		constexpr void operator++() {
			++m_index;
			advance_to_next_index();
		}

		constexpr bool operator==(iterator const& other) const { return &m_set_ref == &other.m_set_ref && m_index == other.m_index; }
		constexpr bool operator!=(iterator const& other) const { return !(*this == other); }

		constexpr Card operator*() const { return static_cast<Card>(m_index); }

	private:
		std::bitset<CardUtils::CARD_COUNT> const& m_set_ref;
		std::size_t m_index { 0 };
	};

	constexpr CardSet() = default;
	constexpr CardSet(std::initializer_list<Card> cards) {
		for (auto card : cards)
			m_set[static_cast<std::size_t>(card)] = true;
		m_size = cards.size();
	}

	constexpr CardSet(std::bitset<CardUtils::CARD_COUNT> set)
	  : m_set(set), m_size(set.count()) {}

	constexpr std::size_t size() const { return m_size; }
	constexpr bool empty() const { return m_size == 0; }
	constexpr bool contains(Card card) const { return m_set[static_cast<std::size_t>(card)]; }

	constexpr bool insert(Card card) {
		if (contains(card))
			return true;

		m_set[static_cast<std::size_t>(card)] = true;
		++m_size;
		return false;
	}

	constexpr void erase(Card card) {
		if (!contains(card))
			return;

		m_set[static_cast<std::size_t>(card)] = false;
		--m_size;
	}

	constexpr void clear() {
		m_set.reset();
		m_size = 0;
	}

	constexpr bool operator==(CardSet const& other) const = default;
	constexpr bool operator!=(CardSet const& other) const = default;

	constexpr CardSet& set_union(CardSet const& other) {
		m_set |= other.m_set;
		m_size = m_set.count();
		return *this;
	}

	static constexpr CardSet intersection(CardSet const& a, CardSet const& b) { return a.m_set & b.m_set; }
	constexpr bool is_subset(CardSet const& other) const { return (m_set & other.m_set) == m_set; }

	constexpr iterator begin() const { return { m_set }; }
	constexpr iterator end() const { return { m_set, m_set.size() }; }

private:
	std::bitset<CardUtils::CARD_COUNT> m_set;
	std::size_t m_size { 0 };
};

};

template<>
struct std::hash<Cluedo::CardSet> {
	std::size_t operator()(Cluedo::CardSet const& set) const noexcept {
		return std::hash<std::bitset<Cluedo::CardUtils::CARD_COUNT>>()(set.m_set);
	}
};
