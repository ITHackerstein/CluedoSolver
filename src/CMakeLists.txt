include(FetchContent)

FetchContent_Declare(
	fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt
	GIT_TAG 10.1.1
	GIT_PROGRESS TRUE
	USES_TERMINAL_DOWNLOAD TRUE
)

FetchContent_MakeAvailable(fmt)

add_executable(Cluedo
	Card.cpp
	Error.cpp
	Player.cpp
	Solver.cpp
	main.cpp
)

target_compile_options(Cluedo PRIVATE
	-Wall
	-Wextra
	-Wshadow
	-Wnon-virtual-dtor
	-Wold-style-cast
	-Wcast-align
	-Wunused
	-Woverloaded-virtual
	-Wconversion
	-Wsign-conversion
	-Wnull-dereference
	-Wdouble-promotion
	-Wformat=2
	-Wimplicit-fallthrough
	-Werror
)
target_include_directories(Cluedo PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(Cluedo PRIVATE fmt::fmt)
