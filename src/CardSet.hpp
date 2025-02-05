#pragma once

#include <bitset>
#include <initializer_list>

#include "Card.hpp"

/// \file CardSet.hpp
/// \brief The file that contains the definition of the \ref Cluedo::CardSet class.

namespace Cluedo {

/// \brief A set of cards.
///
/// This class is an optimization of what would have otherwise been
/// `std::unordered_set<Card>`.
/// Knowing that the number of cards in a Cluedo game is fixed and small
/// we can use a `std::bitset` (which will probably fit in a single 32-bit
/// integer) to store the cards.
class CardSet {
public:
	friend std::hash<CardSet>;

	/// \brief An iterator for the \ref CardSet class.
	class iterator {
	public:
		/// Constructs an iterator that points to the first card of the set.
		///
		/// \param set_ref The reference to the set.
		constexpr iterator(std::bitset<CardUtils::CARD_COUNT> const& set_ref)
		  : m_set_ref(set_ref) {
			advance_to_next_index();
		}

		/// Constructs an iterator that points to the card of the set with the index given.
		/// \note If the card at \a index is not in the set then the iterator will point to the next card in the set.
		///
		/// \param set_ref The reference to the set.
		/// \param index The index of the element of the set.
		constexpr iterator(std::bitset<CardUtils::CARD_COUNT> const& set_ref, std::size_t index)
		  : m_set_ref(set_ref), m_index(index) {
			advance_to_next_index();
		}

		/// If the current index does not point to a card in the set, then it
		/// advances the iterator to the next card in the set.
		constexpr void advance_to_next_index() {
			while (m_index < m_set_ref.size() && !m_set_ref[m_index])
				++m_index;
		}

		/// Advances the iterator to the next card in the set.
		constexpr void operator++() {
			++m_index;
			advance_to_next_index();
		}

		/// Compares two iterators.
		///
		/// \param other The other iterator.
		///
		/// \return `true` if the set they reference is the same and they point to the same card, `false` otherwise.
		constexpr bool operator==(iterator const& other) const { return &m_set_ref == &other.m_set_ref && m_index == other.m_index; }
		/// Compares two iterators.
		///
		/// \param other The other iterator.
		///
		/// \return `false` if the set they reference is the same and they point to the same card, `true` otherwise.
		constexpr bool operator!=(iterator const& other) const { return !(*this == other); }

		/// Returns the card that the iterator points to.
		constexpr Card operator*() const { return static_cast<Card>(m_index); }

	private:
		std::bitset<CardUtils::CARD_COUNT> const& m_set_ref;
		std::size_t m_index { 0 };
	};

	/// Constructs an empty set.
	constexpr CardSet() = default;

	/// Constructs a set with the given cards.
	///
	/// \param cards The list of cards that set will contain.
	constexpr CardSet(std::initializer_list<Card> cards) {
		for (auto card : cards)
			m_set[static_cast<std::size_t>(card)] = true;
		m_size = cards.size();
	}

	/// Constructs a set with the given bitset.
	///
	/// \param set The bitset that the set will contain.
	constexpr CardSet(std::bitset<CardUtils::CARD_COUNT> set)
	  : m_set(set), m_size(set.count()) {}

	/// Returns the number of cards in the set.
	///
	/// \return The number of cards in the set.
	constexpr std::size_t size() const { return m_size; }
	/// Checks if the set is empty.
	///
	/// \return `true` if the set is empty, `false` otherwise.
	constexpr bool empty() const { return m_size == 0; }
	/// Checks if the set contains the given card.
	///
	/// \param card The card to check.
	///
	/// \return `true` if the set contains the card, `false` otherwise.
	constexpr bool contains(Card card) const { return m_set[static_cast<std::size_t>(card)]; }

	/// Inserts a card into the set.
	///
	/// \param card The card to insert.
	///
	/// \return `true` if the card was already in the set, `false` otherwise.
	constexpr bool insert(Card card) {
		if (contains(card))
			return true;

		m_set[static_cast<std::size_t>(card)] = true;
		++m_size;
		return false;
	}

	/// Removes a card from the set.
	///
	/// \param card The card to remove.
	constexpr void erase(Card card) {
		if (!contains(card))
			return;

		m_set[static_cast<std::size_t>(card)] = false;
		--m_size;
	}

	/// Clears the set.
	constexpr void clear() {
		m_set.reset();
		m_size = 0;
	}

	/// Compares two sets.
	///
	/// \param other The other set.
	///
	/// \return `true` if the sets are equal, `false` otherwise.
	constexpr bool operator==(CardSet const& other) const = default;
	/// Compares two sets.
	///
	/// \param other The other set.
	///
	/// \return `true` if the sets are not equal, `false` otherwise.
	constexpr bool operator!=(CardSet const& other) const = default;

	/// Computes the union of two sets.
	/// \note The set that calls this method will be modified.
	///
	/// \param other The other set
	///
	/// \return A reference to the set that called the method.
	constexpr CardSet& set_union(CardSet const& other) {
		m_set |= other.m_set;
		m_size = m_set.count();
		return *this;
	}

	/// Computes the intersection of two sets.
	///
	/// \param a The first set.
	/// \param b The second set.
	///
	/// \return The intersection of the \a a and \a b.
	static constexpr CardSet intersection(CardSet const& a, CardSet const& b) { return a.m_set & b.m_set; }

	/// Checks if the set is a subset of another set.
	///
	/// \param other The other set.
	///
	/// \return `true` if the set is a subset of the other set, `false` otherwise.
	constexpr bool is_subset(CardSet const& other) const { return (m_set & other.m_set) == m_set; }

	/// Returns an iterator to the first card of the set.
	constexpr iterator begin() const { return { m_set }; }

	/// Returns an iterator to the last card of the set.
	constexpr iterator end() const { return { m_set, m_set.size() }; }

private:
	std::bitset<CardUtils::CARD_COUNT> m_set;
	std::size_t m_size { 0 };
};

};

/// Specialization of `std::hash` for the `CardSet` class.
template<>
struct std::hash<Cluedo::CardSet> {
	/// Computesthe hash of a `CardSet` object using the hash of the underlying `std::bitset`.
	std::size_t operator()(Cluedo::CardSet const& set) const noexcept {
		return std::hash<std::bitset<Cluedo::CardUtils::CARD_COUNT>>()(set.m_set);
	}
};
