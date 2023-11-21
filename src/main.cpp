#include "Error.hpp"
#include "Solver.hpp"
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

Result<void, Cluedo::Error> my_main([[maybe_unused]] std::vector<std::string_view>&& arguments) {
	Cluedo::UI::main();

	return {};
}

int main(int argc, char** argv) {
	std::vector<std::string_view> arguments;
	for (int i = 1; i < argc; ++i)
		arguments.emplace_back(argv[i + 1], std::strlen(argv[i + 1]));

	auto maybe_error = my_main(std::move(arguments));
	if (!maybe_error.is_error())
		return EXIT_SUCCESS;

	auto error = maybe_error.release_error();
	fmt::println("[{}] {}", fmt::styled("ERROR", fmt::fg(fmt::color::red)), error);
	return EXIT_FAILURE;
}
