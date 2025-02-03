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
set_target_properties(binary_to_compressed_c PROPERTIES LINK_SEARCH_START_STATIC ON)
set_target_properties(binary_to_compressed_c PROPERTIES LINK_SEARCH_END_STATIC ON)
target_link_options(binary_to_compressed_c PRIVATE -static-libgcc -static-libstdc++)

add_executable(generate_fonts_cpp "${CMAKE_SOURCE_DIR}/src/misc/generate_fonts_cpp.cpp")
set_target_properties(generate_fonts_cpp PROPERTIES LINK_SEARCH_START_STATIC ON)
set_target_properties(generate_fonts_cpp PROPERTIES LINK_SEARCH_END_STATIC ON)
target_link_options(generate_fonts_cpp PRIVATE -static-libgcc -static-libstdc++)

add_executable(generate_langs_cpp "${CMAKE_SOURCE_DIR}/src/misc/generate_langs_cpp.cpp")
set_target_properties(generate_langs_cpp PROPERTIES LINK_SEARCH_START_STATIC ON)
set_target_properties(generate_langs_cpp PROPERTIES LINK_SEARCH_END_STATIC ON)
target_link_options(generate_langs_cpp PRIVATE -static-libgcc -static-libstdc++)

set(FONT_IDS ibm_plex_sans_regular fa_regular fa_solid)
set(FONT_NAMES "IBMPlexSans-Medium" "fa-regular-400" "fa-solid-900")

foreach(FONT_ID FONT_NAME IN ZIP_LISTS FONT_IDS FONT_NAMES)
	add_custom_command(
		OUTPUT "${CMAKE_SOURCE_DIR}/src/fonts/${FONT_NAME}.cpp"
		DEPENDS binary_to_compressed_c
		DEPENDS "${CMAKE_SOURCE_DIR}/res/fonts/${FONT_NAME}.ttf"
		COMMAND binary_to_compressed_c "${CMAKE_SOURCE_DIR}/res/fonts/${FONT_NAME}.ttf" ${FONT_ID} > "${CMAKE_SOURCE_DIR}/src/fonts/${FONT_NAME}.cpp"
	)
endforeach()

set(FONT_FILES ${FONT_NAMES})
list(TRANSFORM FONT_FILES PREPEND "${CMAKE_SOURCE_DIR}/src/fonts/")
list(TRANSFORM FONT_FILES APPEND ".cpp")

add_custom_command(
	OUTPUT "${CMAKE_SOURCE_DIR}/src/fonts/fonts.cpp"
	DEPENDS generate_fonts_cpp
	COMMAND generate_fonts_cpp ${FONT_NAMES} > "${CMAKE_SOURCE_DIR}/src/fonts/fonts.cpp"
)

add_custom_target(Fonts ALL DEPENDS ${FONT_FILES} "${CMAKE_SOURCE_DIR}/src/fonts/fonts.cpp")

set(LANGUAGE_IDS en it)
set(LANGUAGE_NAMES "English" "Italian")
foreach(LANGUAGE_ID IN LISTS LANGUAGE_IDS)
	add_custom_command(
		OUTPUT "${CMAKE_SOURCE_DIR}/src/lang/${LANGUAGE_ID}.cpp"
		DEPENDS binary_to_compressed_c
		DEPENDS "${CMAKE_SOURCE_DIR}/res/lang/${LANGUAGE_ID}.json"
		COMMAND binary_to_compressed_c -nocompress "${CMAKE_SOURCE_DIR}/res/lang/${LANGUAGE_ID}.json" lang_${LANGUAGE_ID} > "${CMAKE_SOURCE_DIR}/src/lang/${LANGUAGE_ID}.cpp"
	)
endforeach()

set(LANGUAGE_FILES ${LANGUAGE_IDS})
list(TRANSFORM LANGUAGE_FILES PREPEND "${CMAKE_SOURCE_DIR}/src/lang/")
list(TRANSFORM LANGUAGE_FILES APPEND ".cpp")

add_custom_command(
	OUTPUT "${CMAKE_SOURCE_DIR}/src/lang/langs.cpp"
	DEPENDS generate_langs_cpp
	COMMAND generate_langs_cpp ${LANGUAGE_IDS} ${LANGUAGE_NAMES} > "${CMAKE_SOURCE_DIR}/src/lang/langs.cpp"
)

add_custom_target(Languages ALL DEPENDS ${LANGUAGE_FILES} "${CMAKE_SOURCE_DIR}/src/lang/langs.cpp")

add_executable(CluedoSolver
	Card.cpp
	Error.cpp
	Player.cpp
	Solver.cpp
	LanguageStrings.cpp
	${IMGUI_SOURCE_FILES}
	main.cpp
)

add_dependencies(CluedoSolver Fonts Languages)

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

target_link_libraries(CluedoSolver
	PRIVATE fmt::fmt
	PRIVATE nlohmann_json::nlohmann_json
	PRIVATE SDL2::SDL2-static
	PRIVATE OpenGL::GL
)

if (TARGET SDL2::SDL2main)
	target_link_libraries(CluedoSolver PRIVATE SDL2::SDL2main)
endif()

set_target_properties(CluedoSolver PROPERTIES LINK_SEARCH_START_STATIC ON)
set_target_properties(CluedoSolver PROPERTIES LINK_SEARCH_END_STATIC ON)
target_link_options(CluedoSolver PRIVATE -static-libgcc -static-libstdc++)
