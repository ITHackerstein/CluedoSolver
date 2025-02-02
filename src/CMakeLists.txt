include(FetchContent)

FetchContent_Declare(
	fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt
	GIT_TAG 11.1.3
)

FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
	json
	URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)

FetchContent_MakeAvailable(json)

find_package(OpenGL REQUIRED)
add_subdirectory("${CMAKE_SOURCE_DIR}/src/libs/SDL2")
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/src/libs/imgui")
file(GLOB IMGUI_SOURCE_FILES "${IMGUI_DIR}/*.cpp")
set(IMGUI_SOURCE_FILES ${IMGUI_SOURCE_FILES}
	"${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp"
	"${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
	"${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp"
)

add_executable(binary_to_compressed_c "${IMGUI_DIR}/misc/fonts/binary_to_compressed_c.cpp")

add_custom_command(
	OUTPUT "${CMAKE_SOURCE_DIR}/src/fonts/IBMPlexSans-Medium.cpp"
	DEPENDS binary_to_compressed_c
	COMMAND binary_to_compressed_c "${CMAKE_SOURCE_DIR}/res/fonts/IBMPlexSans-Medium.ttf" ibm_plex_sans_regular > "${CMAKE_SOURCE_DIR}/src/fonts/IBMPlexSans-Medium.cpp"
)

add_custom_command(
	OUTPUT "${CMAKE_SOURCE_DIR}/src/fonts/fa-regular-400.cpp"
	DEPENDS binary_to_compressed_c
	COMMAND binary_to_compressed_c "${CMAKE_SOURCE_DIR}/res/fonts/fa-regular-400.ttf" fa_regular > "${CMAKE_SOURCE_DIR}/src/fonts/fa-regular-400.cpp"
)

add_custom_command(
	OUTPUT "${CMAKE_SOURCE_DIR}/src/fonts/fa-solid-900.cpp"
	DEPENDS binary_to_compressed_c
	COMMAND binary_to_compressed_c "${CMAKE_SOURCE_DIR}/res/fonts/fa-solid-900.ttf" fa_solid > "${CMAKE_SOURCE_DIR}/src/fonts/fa-solid-900.cpp"
)

add_custom_target(FontFiles ALL DEPENDS
	"${CMAKE_SOURCE_DIR}/src/fonts/IBMPlexSans-Medium.cpp"
	"${CMAKE_SOURCE_DIR}/src/fonts/fa-regular-400.cpp"
	"${CMAKE_SOURCE_DIR}/src/fonts/fa-solid-900.cpp"
)

add_executable(CluedoSolver
	Card.cpp
	Error.cpp
	Player.cpp
	Solver.cpp
	Strings.cpp
	${IMGUI_SOURCE_FILES}
	main.cpp
)

target_compile_options(CluedoSolver PRIVATE
	-std=gnu++23
	-Wall
	-Wextra
	-Wshadow
	-Werror
)
target_include_directories(CluedoSolver PUBLIC
	${CMAKE_SOURCE_DIR}/src/libs/PCG/
	${IMGUI_DIR}
	${IMGUI_DIR}/backends
	${IMGUI_DIR}/misc/cpp
)

if (TARGET SDL2::SDL2main)
	target_link_libraries(CluedoSolver PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(CluedoSolver
	PRIVATE fmt::fmt
	PRIVATE nlohmann_json::nlohmann_json
	PRIVATE OpenGL::GL
	PRIVATE SDL2::SDL2-static
)
