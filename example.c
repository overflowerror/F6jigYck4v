#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <inttypes.h>

#include "lib/sfuid.h"

#define error(...) fprintf(stderr, __VA_ARGS__)

void help(const char* name) {
	printf("SYNOPSIS: %s [OPTIONS] INPUT\n\n", name);
	printf("options: \n");
	printf("   -e          encode INPUT (default)\n");
	printf("   -d          decode INPUT\n");
	printf("   -l  LENGTH  set the length (default: 10)\n");
	printf("   -c  CHARSET set the charset (default: a-zA-Z0-9)\n");
	printf("   -o  OFFSET  set the offset (default: 5)\n");
	printf("   -h          show this help message\n\n");
}

int main(int argc, char** argv) {
	bool decodeMode = false;

	int opt;
	char* endptr;

	sfuid_settings_t settings = sfuid_default_settings;

	while((opt = getopt(argc, argv, "o:c:l:dehy")) != -1) {
		switch(opt) {
			case 'o':
				settings.offset = strtol(optarg, &endptr, 10);
				if (*endptr != 0) {
					error("Argument for -o has to be a number.\nTry -h to get help.\n");
					exit(2);
				}
				break;
			case 'l':
				settings.length = strtol(optarg, &endptr, 10);
				if (*endptr != 0) {
					error("Argument of -l has to be a number.\nTry -h to get help.\n");
					exit(2);
				}
				break;
			case 'c':
				settings.charset = optarg;
				break;
			case 'e':
				decodeMode = false;
				break;
			case 'd':
				decodeMode = true;
				break;
			case 'h':
				help(argv[0]);
				exit(0);
			case 'y':
				settings.charset = "0-9A-Za-z-_";
				break;
			default:
				error("Unknown option.\nTry -h to get help.\n");
				exit(2);
		}
	}

	if (optind >= argc) {
		error("Expected data after options.\nTry -h to get help.\n");
		exit(2);
	}

	int tmp;

	tmp = sfuid_init(settings);
	if (tmp != SFUID_ERROR_NONE) {
		error("Error: %s\n", sfuid_error(tmp));
		exit(4);
	}

	if (decodeMode) {
		const char* string = argv[optind];
	
		uint64_t value;

		tmp = sfuid_decode(string, &value);
		if (tmp != SFUID_ERROR_NONE) {
			error("Error: %s\n", sfuid_error(tmp));
			exit(4);
		}

		printf("%" PRIu64 "\n", value);
	} else {
		uint64_t value = strtoull(argv[optind], &endptr, 10);
		if (*endptr != 0) {
			error("Value has to be a number for encoding.\n");
			exit(2);
		}
		if (value == ULLONG_MAX && errno == ERANGE) {
			error("Value overflows the internal datatype.\n");
			exit(2);
		}

		char* string = malloc(settings.length + 1);

		tmp = sfuid_encode(value, string);

		if (tmp != SFUID_ERROR_NONE) {
			error("Error: %s\n", sfuid_error(tmp));
			exit(4);
		}

		printf("%s\n", string);
		free(string);
	}
}
