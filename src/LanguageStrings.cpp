#include "LanguageStrings.hpp"

#include "lang/langs.cpp"

#include <cassert>
#include <fmt/core.h>
#include <ranges>

using json = nlohmann::json;
using namespace std::literals;

namespace Cluedo {

std::unique_ptr<LanguageStrings> LanguageStrings::s_instance = nullptr;

std::vector<LanguageStrings::Language> LanguageStrings::s_languages = {
#define _ENUMERATE_LANGUAGE(language_id, language_name) { #language_id##sv, #language_name##sv, std::string_view { reinterpret_cast<char const*>(lang_##language_id##_data), lang_##language_id##_size } },
	_ENUMERATE_LANGUAGES
#undef _ENUMERATE_LANGUAGE
};

LanguageStrings& LanguageStrings::the() {
	if (s_instance == nullptr) {
		s_instance = std::unique_ptr<LanguageStrings>(new LanguageStrings);
		s_instance->set_language("en"sv);
	}

	return *s_instance;
}

void LanguageStrings::set_language(std::string_view language) {
	if (m_current_language_id == language)
		return;

	auto it = std::find_if(s_languages.begin(), s_languages.end(), [language](Language const& l) { return l.id == language; });
	assert(it != s_languages.end() && "Language not found!");

	try {
		m_strings = json::parse(it->data.begin(), it->data.end());
		assert(m_strings.is_object() && "Language file isn't an object!");
		m_current_language_id = language;
	} catch (json::exception const& exception) {
		assert(false && "Error while parsing language file");
	}
}

std::string_view LanguageStrings::get_string(std::string_view key) const {
	auto const* obj = &m_strings;
	for (auto part : std::ranges::views::split(key, "."sv)) {
		auto it = obj->find(std::string_view { part });
		assert(it != obj->end() && "Invalid key provided!");
		obj = &(*it);
	}

	assert(obj->is_string() && "Invalid key provided!");
	return obj->get<std::string_view>();
}

};
