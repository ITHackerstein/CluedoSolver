#pragma once

#include <memory>
#include <nlohmann/json.hpp>

/// \file LanguageStrings.hpp
/// \brief The file that contains the definition of the \ref Cluedo::LanguageStrings class.

namespace Cluedo {

/// The class that contains the strings of the application which can be translated.
/// \note This class is a singleton.
class LanguageStrings {
public:
	/// \brief A struct that contains the data of a language.
	struct Language {
		std::string_view id;   ///< An identifier used for the language.
		std::string_view name; ///< The name of the language.
		std::string_view data; ///< The JSON data of the language.
	};

	/// Returns the instance of the class.
	///
	/// \return The instance of the class.
	static LanguageStrings& the();

	/// Returns the vector containing the available languages for the application.
	///
	/// \return The vector containing the available languages for the application.
	static std::vector<Language> const& languages() { return s_languages; }
	/// Returns the ID of the current language.
	///
	/// \return The ID of the current language.
	std::string_view current_language_id() const { return m_current_language_id; }
	/// Sets the language of the application to the one with the given ID.
	///
	/// \param id The ID of the language to use.
	void set_language(std::string_view id);

	/// Returns the string with the given key.
	/// \note The function will fail if the key is not found.
	///
	/// \param key The key of the string.
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

/// \def LS(key)
/// \brief A macro that is used to get a string from the \ref Cluedo::LanguageStrings.
///
/// \param key The key of the string.
#define LS(key) (Cluedo::LanguageStrings::the().get_string((key)))
/// \def CSTR(s)
/// \brief A macro that is used to get a `char*` from a string.
///
/// The main reason why this macro exists is that the
/// <a href="https://github.com/ocornut/imgui/">Dear ImGui</a> library doesn't
/// support `std::string` directly.
///
/// \param s The string to convert.
#define CSTR(s) (std::string { (s) }.c_str())

};
