#include <stdio.h>

int main(int argc, char** argv) {
	(void)argc;

	while (*(++argv) != NULL) {
		printf("#include \"%s.cpp\"\n", *argv);
	}

	return 0;
}
