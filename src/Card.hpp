#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace Cluedo {

enum class CardCategory : std::uint8_t {
	Suspect = 0,
	Weapon = 6,
	Room = 12
};

std::string_view format_as(CardCategory);

#define _ENUMERATE_SUSPECTS \
	_ENUMERATE_CARD(Green)    \
	_ENUMERATE_CARD(Mustard)  \
	_ENUMERATE_CARD(Orchid)   \
	_ENUMERATE_CARD(Peacock)  \
	_ENUMERATE_CARD(Plum)     \
	_ENUMERATE_CARD(Scarlet)

#define _ENUMERATE_WEAPONS     \
	_ENUMERATE_CARD(Candlestick) \
	_ENUMERATE_CARD(Knife)       \
	_ENUMERATE_CARD(Pipe)        \
	_ENUMERATE_CARD(Pistol)      \
	_ENUMERATE_CARD(Rope)        \
	_ENUMERATE_CARD(Wrench)

#define _ENUMERATE_ROOMS       \
	_ENUMERATE_CARD(BallardRoom) \
	_ENUMERATE_CARD(Ballroom)    \
	_ENUMERATE_CARD(DiningRoom)  \
	_ENUMERATE_CARD(Greenhouse)  \
	_ENUMERATE_CARD(Hall)        \
	_ENUMERATE_CARD(Kitchen)     \
	_ENUMERATE_CARD(Library)     \
	_ENUMERATE_CARD(Lounge)      \
	_ENUMERATE_CARD(Study)

#define _ENUMERATE_CARDS \
	_ENUMERATE_SUSPECTS    \
	_ENUMERATE_WEAPONS     \
	_ENUMERATE_ROOMS

enum class Card : std::uint8_t {
#define _ENUMERATE_CARD(x) x,
	_ENUMERATE_CARDS
#undef _ENUMERATE_CARD
	  _Count
};

std::string_view format_as(Card);

struct CardUtils {
	static constexpr std::size_t CARD_COUNT = static_cast<std::size_t>(Card::_Count);

	static constexpr std::array card_categories { CardCategory::Suspect, CardCategory::Weapon, CardCategory::Room };

	static constexpr CardCategory card_category(Card card) {
		auto card_u8 = static_cast<std::uint8_t>(card);

		if (card_u8 < static_cast<std::uint8_t>(CardCategory::Weapon))
			return CardCategory::Suspect;

		if (card_u8 < static_cast<std::uint8_t>(CardCategory::Room))
			return CardCategory::Weapon;

		return CardCategory::Room;
	}

	struct CardIterator {
		std::uint8_t index;
		// cppcheck-suppress noExplicitConstructor
		constexpr CardIterator(std::uint8_t i)
		  : index(i) {}

		constexpr Card operator*() const { return static_cast<Card>(index); }
		constexpr bool operator!=(CardIterator const& other) const { return index != other.index; }
		constexpr void operator++() { ++index; }
	};

	struct cards {
		constexpr CardIterator begin() const { return 0; }
		constexpr CardIterator end() const { return static_cast<std::uint8_t>(Card::_Count); }
	};

	struct cards_per_category {
		CardCategory category;
		explicit constexpr cards_per_category(CardCategory c)
		  : category(c) {}

		constexpr CardIterator begin() const { return static_cast<std::uint8_t>(category); }
		constexpr CardIterator end() const {
			std::uint8_t count;
			switch (category) {
			case CardCategory::Suspect:
				count = 6;
				break;
			case CardCategory::Weapon:
				count = 6;
				break;
			case CardCategory::Room:
				count = 9;
				break;
			}

			return static_cast<std::uint8_t>(category) + count;
		}
	};
};

};
