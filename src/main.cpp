#include "Error.hpp"
#include "Solver.hpp"
#include "Strings.hpp"
#include "UI.hpp"
#include "Utils/Result.hpp"

#include <cstdlib>
#include <cstring>
#include <fmt/color.h>
#include <fmt/core.h>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string_view>
#include <vector>

Result<void, std::string> my_main([[maybe_unused]] std::vector<std::string_view>&& arguments) {
	using namespace std::literals;

	auto file_name = "res/en.json"sv;
	if (arguments.size() > 0)
		file_name = arguments.at(0);

	TRY(Cluedo::Strings::load_from_file(file_name));

	Cluedo::UI::main();

	return {};
}

int main(int argc, char** argv) {
	std::vector<std::string_view> arguments;
	for (int i = 1; i < argc; ++i)
		arguments.emplace_back(argv[i], std::strlen(argv[i]));

	auto maybe_error = my_main(std::move(arguments));
	if (!maybe_error.is_error())
		return EXIT_SUCCESS;

	auto error = maybe_error.release_error();
	fmt::println("[{}] {}", fmt::styled("ERROR", fmt::fg(fmt::color::red)), error);
	return EXIT_FAILURE;
}
