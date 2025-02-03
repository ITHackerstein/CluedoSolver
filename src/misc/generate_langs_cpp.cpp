#include <stdio.h>

int main(int argc, char** argv) {
	++argv;
	--argc;

	int language_count = argc / 2;
	for (int i = 0; i < language_count; ++i) {
		printf("#include \"%s.cpp\"\n", argv[i]);
	}

	printf("#define _ENUMERATE_LANGUAGES \\\n");
	for (int i = 0; i < language_count; ++i) {
		printf("_ENUMERATE_LANGUAGE(%s, %s)", argv[i], argv[i + language_count]);
		if (i < language_count - 1)
			printf(" \\\n");
	}
	printf("\n");

	return 0;
}
