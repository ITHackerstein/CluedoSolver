#pragma once

#include <memory>
#include <nlohmann/json.hpp>

namespace Cluedo {

class LanguageStrings {
public:
	struct Language {
		std::string_view id;
		std::string_view name;
		std::string_view data;
	};

	static LanguageStrings& the();

	static std::vector<Language> const& languages() { return s_languages; }
	std::string_view current_language_id() const { return m_current_language_id; }
	void set_language(std::string_view);

	std::string_view get_string(std::string_view key) const;

private:
	static std::unique_ptr<LanguageStrings> s_instance;
	static std::vector<Language> s_languages;

	LanguageStrings() = default;

	LanguageStrings(LanguageStrings const&) = delete;
	LanguageStrings& operator=(LanguageStrings const&) = delete;

	LanguageStrings(LanguageStrings&&) = delete;
	LanguageStrings& operator=(LanguageStrings&&) = delete;

	std::string_view m_current_language_id;
	nlohmann::json m_strings;
};

};
