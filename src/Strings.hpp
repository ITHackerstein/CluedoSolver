#pragma once

#include <memory>
#include <nlohmann/json.hpp>

#include "Error.hpp"
#include "Utils/Result.hpp"

namespace Cluedo {

class Strings {
public:
	static Result<void, std::string> load_from_file(std::string_view file_name);
	static Strings& the();

	std::string_view get_string(std::string_view key) const;

private:
	static std::unique_ptr<Strings> s_instance;

	Strings(nlohmann::json&& strings):
		m_strings(std::move(strings)) {}

	static Result<void, std::string> find_error_strings(nlohmann::json const& strings);
	static Result<void, std::string> find_card_category_strings(nlohmann::json const& strings);
	static Result<void, std::string> find_card_strings(nlohmann::json const& strings);
	static Result<void, std::string> find_solver_strings(nlohmann::json const& strings);
	static Result<void, std::string> find_ui_strings(nlohmann::json const& strings);

	Strings(Strings const&) = delete;
	Strings& operator=(Strings const&) = delete;

	Strings(Strings&&) = delete;
	Strings& operator=(Strings&&) = delete;

	nlohmann::json m_strings;
};

};
