add_executable(Cluedo
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
	-Wpedantic
	-Wconversion
	-Wsign-conversion
	-Wnull-dereference
	-Wdouble-promotion
	-Wformat=2
	-Wimplicit-fallthrough
	-Werror
)
find_package(fmt)
target_link_libraries(Cluedo fmt::fmt)
