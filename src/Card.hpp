#pragma once

#include <array>
#include <cstdint>
#include <string_view>

/// \file Card.hpp
/// The file that contains data about Cluedo cards.

namespace Cluedo {

/// \brief The categories of the cards in Cluedo.
enum class CardCategory : std::uint8_t {
	Suspect = 0, ///< Suspect cards.
	Weapon = 6,  ///< Weapon cards.
	Room = 12    ///< Room cards.
};

/// Formats the card category as a string.
/// \note This function is meant to be used by the
/// <a href="https://github.com/fmtlib/fmt">{fmt}</a> library to format the
/// card categories correctly.
std::string_view format_as(CardCategory);

/// \def _ENUMERATE_SUSPECTS
/// \brief Enumerates the suspect cards.
#define _ENUMERATE_SUSPECTS \
	_ENUMERATE_CARD(Green)    \
	_ENUMERATE_CARD(Mustard)  \
	_ENUMERATE_CARD(Orchid)   \
	_ENUMERATE_CARD(Peacock)  \
	_ENUMERATE_CARD(Plum)     \
	_ENUMERATE_CARD(Scarlet)

/// \def _ENUMERATE_WEAPONS
/// \brief Enumerates the weapon cards.
#define _ENUMERATE_WEAPONS     \
	_ENUMERATE_CARD(Candlestick) \
	_ENUMERATE_CARD(Knife)       \
	_ENUMERATE_CARD(Pipe)        \
	_ENUMERATE_CARD(Pistol)      \
	_ENUMERATE_CARD(Rope)        \
	_ENUMERATE_CARD(Wrench)

/// \def _ENUMERATE_ROOMS
/// \brief Enumerates the room cards.
#define _ENUMERATE_ROOMS        \
	_ENUMERATE_CARD(BilliardRoom) \
	_ENUMERATE_CARD(Ballroom)     \
	_ENUMERATE_CARD(DiningRoom)   \
	_ENUMERATE_CARD(Greenhouse)   \
	_ENUMERATE_CARD(Hall)         \
	_ENUMERATE_CARD(Kitchen)      \
	_ENUMERATE_CARD(Library)      \
	_ENUMERATE_CARD(Lounge)       \
	_ENUMERATE_CARD(Study)

/// \def _ENUMERATE_CARDS
/// \brief Enumerates all the cards.
#define _ENUMERATE_CARDS \
	_ENUMERATE_SUSPECTS    \
	_ENUMERATE_WEAPONS     \
	_ENUMERATE_ROOMS

/// \brief All the cards in Cluedo.
enum class Card : std::uint8_t {
#define _ENUMERATE_CARD(x) x,
	_ENUMERATE_CARDS
#undef _ENUMERATE_CARD
	  _Count
};

/// Formats the card as a string.
/// \note This function is meant to be used by the
/// <a href="https://github.com/fmtlib/fmt">{fmt}</a> library to format the
/// cards correctly.
std::string_view format_as(Card);

/// \brief A series of utilities for the cards.
struct CardUtils {
	/// The number of cards in Cluedo.
	static constexpr std::size_t CARD_COUNT = static_cast<std::size_t>(Card::_Count);
	/// The categories of the cards stored as an array.
	static constexpr std::array card_categories { CardCategory::Suspect, CardCategory::Weapon, CardCategory::Room };

	/// Returns the category of a card.
	///
	/// \param card The card of which to get the category.
	static constexpr CardCategory card_category(Card card) {
		auto card_u8 = static_cast<std::uint8_t>(card);

		if (card_u8 < static_cast<std::uint8_t>(CardCategory::Weapon))
			return CardCategory::Suspect;

		if (card_u8 < static_cast<std::uint8_t>(CardCategory::Room))
			return CardCategory::Weapon;

		return CardCategory::Room;
	}

	/// An iterator for cards.
	struct CardIterator {
		std::uint8_t index; ///< The index of the card.

		/// Constructs an iterator that points to the card with the given index.
		///
		/// \param i The index of the card.
		constexpr CardIterator(std::uint8_t i)
		  : index(i) {}

		/// Returns the card that the iterator points to.
		///
		/// \return The card that the iterator points to.
		constexpr Card operator*() const { return static_cast<Card>(index); }

		/// Compares two iterators.
		///
		/// \param other The other iterator.
		///
		/// \return `true` if the iterators don't point to the same card, `false` otherwise.
		constexpr bool operator!=(CardIterator const& other) const { return index != other.index; }

		/// Advances the iterator to the next card.
		constexpr void operator++() { ++index; }
	};

	/// Helper class for iterator on all the cards.
	struct cards {
		/// Returns an iterator that points to the first card.
		///
		/// \return The iterator that points to the first card.
		constexpr CardIterator begin() const { return 0; }
		/// Returns an iterator that points to the last card.
		///
		/// \return The iterator that points to the last card.
		constexpr CardIterator end() const { return static_cast<std::uint8_t>(Card::_Count); }
	};

	/// Helper class for iterator on all the cards of a category.
	struct cards_per_category {
		CardCategory category; ///< The category of the cards to iterate on.

		/// Constructs the helper class.
		explicit constexpr cards_per_category(CardCategory c)
		  : category(c) {}

		/// Returns the number of cards in the category.
		constexpr std::uint8_t count() const {
			switch (category) {
			case CardCategory::Suspect:
				return 6;
			case CardCategory::Weapon:
				return 6;
			case CardCategory::Room:
				return 9;
			}

			return 0;
		}

		/// Returns an iterator that points to the first card of the category.
		///
		/// \return The iterator that points to the first card of the category.
		constexpr CardIterator begin() const { return static_cast<std::uint8_t>(category); }
		/// Returns an iterator that points to the last card of the category.
		///
		/// \return The iterator that points to the last card of the category.
		constexpr CardIterator end() const { return static_cast<std::uint8_t>(category) + count(); }
	};
};

};
