#pragma once

#include <array>
#include <cstdint>
#include <unordered_set>

namespace Cluedo {

enum class CardCategory : std::uint8_t {
	Suspect = 0,
	Weapon = 6,
	Room = 12
};

enum class Card : std::uint8_t {
	// Suspects
	Green,
	Mustard,
	Orchid,
	Peacock,
	Plum,
	Scarlet,
	// Weapons
	Candlestick,
	Knife,
	Pipe,
	Pistol,
	Rope,
	Wrench,
	// Rooms
	BallardRoom,
	Ballroom,
	DiningRoom,
	Greenhouse,
	Hall,
	Kitchen,
	Library,
	Lounge,
	Study,
	// Used to count cards
	_Count
};

struct CardUtils {
	static constexpr std::size_t CARD_COUNT = static_cast<std::size_t>(Card::_Count);

	static constexpr std::array card_categories { CardCategory::Suspect, CardCategory::Weapon, CardCategory::Room };

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

template<>
struct std::hash<std::unordered_set<Cluedo::Card>> {
	std::size_t operator()(std::unordered_set<Cluedo::Card> const& set) const noexcept {
		std::size_t result = 0;
		for (auto const& card : set)
			result ^= std::hash<std::uint8_t> {}(static_cast<std::uint8_t>(card));
		return result;
	}
};
