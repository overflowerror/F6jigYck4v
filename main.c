#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef __uint128_t uint128_t;

char* charset = "a-zA-Z0-9";
unsigned int resultLength = 10;
unsigned int offset = 5;
bool verbose = false;

#define verbose(...) if (verbose) printf(__VA_ARGS__);
#define error(...) fprintf(stderr, __VA_ARGS__)

int convertToString(char* string, unsigned long long number) {
	unsigned charsetLength = strlen(charset);

	int length = 0;
	do {
		string[length++] = charset[number % charsetLength];
		number /= charsetLength;
	} while(number > 0);

	for(; length < resultLength; length++) {
		string[length] = charset[0];
	}

	for(int i = 0; i < length / 2; i++) {
		string[i] ^= string[length - i - 1];
		string[length - i - 1] ^= string[i];
		string[i] ^= string[length - i - 1];
	}

	string[length] = '\0';

	return length;
}

unsigned long long convertFromString(const char* string) {
	unsigned long long result = 0;

	int stringLength = strlen(string);
	int charsetLength = strlen(charset);

	for (int i = 0; i < stringLength; i++) {
		bool found = false;
		for(int j = 0; j < charsetLength; j++) {
			if (charset[j] == string[i]) {
				found = true;
				unsigned long long val = ((unsigned long long) pow(charsetLength, stringLength - i - 1));
				//printf("position %d, digit %d, value %d\n", i, j, val);;
				result += ((unsigned long long) j) * val;
				break;
			}
		}
		if (!found) {
			error("Parsing error.\nUnexpected character '%c'.\n", string[i]);
			exit(3);
		}
	}

	return result;
}

unsigned long long primelist[] = {
	1, 1, 1, 3, 7, 13, 29, 61, 113, 251, 509, 1021, 2039, 4093, 8179, 16381,
	32749, 65521, 131063, 262139, 524269, 1048573, 2097143, 4194301,
	8388593, 16777213, 33554393, 67108859, 134217689, 268435399, 
	536870909, 1073741789, 2147483629ll, 4294967291ll, 8589934583ll, 
	17179869143ll, 34359738337ll, 68719476731ll, 137438953447ll, 
	274877906899ll, 549755813881ll, 1099511627689ll, 2199023255531ll, 
	4398046511093ll, 8796093022151ll, 17592186044399ll, 
	35184372088777ll, 70368744177643ll, 140737488355213ll,
	281474976710597ll, 562949953421231ll, 1125899906842597ll,
	2251799813685119ll, 4503599627370449ll, 9007199254740881ll, 
	18014398509481951ll, 36028797018963913ll, 72057594037927931ll, 
	144115188075855859ll, 288230376151711717ll, 576460752303423433ll,
	1152921504606846883ll, 2305843009213693921ll // 61
};

int findMax(unsigned long long theoreticalMax) {
	int result = 0;
	unsigned long long tmp = 0;
	while(true) {
		if (tmp >= theoreticalMax)
			break;
		tmp = 1ll << ++result;
	}
	return result - 1;
}

unsigned long long findPrime(int maxBit) {
	int primelistLength = (sizeof primelist) / (sizeof (long long));
	if (maxBit >= primelistLength)
		return primelist[primelistLength - 1];

	return primelist[maxBit - 1];
}

unsigned long long encode(unsigned long long value, unsigned long long prime, unsigned long long maxValue) {
	uint128_t result = value;

	result *= prime;

	result &= (maxValue - 1);

	return (unsigned long long) result;
}

unsigned long long multiplicativeInverse(unsigned long long a, unsigned long long b) {
	/*
	 * I didn't write this. 
	 * I got this from https://rosettacode.org/wiki/Modular_inverse#C 
	 * and adopted it to use long long data types.
	 */
	long long b0 = b, t, q;
	long long x0 = 0, x1 = 1;
	if (b == 1)
		return 1;
	while (a > 1) {
		q = a / b;
		t = b, b = a % b, a = t;
		t = x0, x0 = x1 - q * x0, x1 = t;
	}
	if (x1 < 0)
		x1 += b0;
	return x1;
}

unsigned long long decode(unsigned long long value, unsigned long long prime, unsigned long long maxValue) {
	/* 
	 * (x * p) mod m = v
	 * (x * p) == v   (mod m)
	 * Def: i = multiplicative inverse of p
	 * => x = v * i
	 * => p * i == i  (mod m)
	 */

	uint128_t tmp = value;
	tmp *= multiplicativeInverse(prime, maxValue);
	tmp &= (maxValue - 1);

	return (unsigned long long) tmp;
}

typedef struct settings {
	unsigned long long max;
	unsigned long long prime;
} settings_t;

settings_t getSettings() {
	verbose("charset length: %d\n", strlen(charset));
	double maxValueDouble = pow(strlen(charset), resultLength);
	unsigned long long maxValue;
	int maxBits; 

	if (maxValueDouble > pow(2, 64)) {
		verbose("Max value %.0lf > 2^64, caping to 63 bits.\n", maxValueDouble);
		maxBits = 63;
	} else {
		maxValue = (unsigned long long) maxValueDouble;
		verbose("theoer max: %llu\n", maxValue);
		maxBits = findMax(maxValue);

		if (maxBits <= 0) {
			error("Length or charset is too big. This should not have happend.\n");
			exit(3);
		}
	}

	maxValue = 1ll << maxBits;

	verbose("used bits:  %d\n", maxBits);
	verbose("max:        %llu\n", maxValue);
	verbose("usage:      %.1f%\n", (100.0 * maxValue / maxValueDouble));

	unsigned long long prime = findPrime(maxBits);

	verbose("prime:      %llu\n", prime);

	return (settings_t) {.max = maxValue, .prime = prime};
}

void toString(char* string, unsigned long long value, settings_t settings) {
	value = encode(value, settings.prime, settings.max);
	verbose("encoded:    %llu\n", value);
	convertToString(string, value);
}

unsigned long long fromString(const char* string, settings_t settings) {
	verbose("test\n");
	unsigned long long result = convertFromString(string);
	verbose("encoded:    %llu\n", result);
	result = decode(result, settings.prime, settings.max);
	return result;
}

struct {
	const char* needle;
	const char* replacement;
} shortcuts[] = {
	{
		.needle = "a-z",
		.replacement = "abcdefghijklmnopqrstuvwxyz"
	},
	{
		.needle = "A-Z",
		.replacement = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	}, 
	{
		.needle = "0-9",
		.replacement = "0123456789"
	}
};

void prepareCharset() {
	verbose("prepareing charset\n");

	int size = strlen(charset);
	char* tmp;

	for(int i = 0; i < ((sizeof shortcuts) / (sizeof shortcuts[0])); i++) {
		tmp = charset;
		while((tmp = strstr(tmp, shortcuts[i].needle)) != NULL) {
			tmp++;
			size += strlen(shortcuts[i].replacement) - strlen(shortcuts[i].needle);
		}
	}

	char* inflatedCharset = malloc(size + 1);
	strcpy(inflatedCharset, charset);

	while(true) {
		char* first = NULL;
		const char* replace;
	
		for (int i = 0; i < ((sizeof shortcuts) / (sizeof shortcuts[0])); i++) {
			tmp = strstr(inflatedCharset, shortcuts[i].needle);
			if ((tmp != NULL) && ((first == NULL) || (tmp < first))) {
				first = tmp;
				replace = shortcuts[i].replacement;
			}
		}	

		if (first == NULL)
			break;

		memmove(first + strlen(replace), first + 3, strlen(first) + 1);
		memcpy(first, replace, strlen(replace));
	}

	//verbose("full charset: %s\n", inflatedCharset);

	charset = inflatedCharset;
}

void help(const char* name) {
	printf("SYNOPSIS: %s [OPTIONS] INPUT\n\n", name);
	printf("options: \n");
	printf("   -e          encode INPUT (default)\n");
	printf("   -d          decode INPUT\n");
	printf("   -l  LENGTH  set the length (default: 10)\n");
	printf("   -c  CHARSET set the charset (default: a-zA-Z0-9)\n");
	printf("   -o  OFFSET  set the offset (default: 5)\n");
	printf("   -v          enable verbose mode\n");
	printf("   -h          show this help message\n\n");
}

int main(int argc, char** argv) {
	bool decodeMode = false;

	int opt;
	char* endptr;

	while((opt = getopt(argc, argv, "o:c:l:devh")) != -1) {
		switch(opt) {
			case 'o':
				offset = strtol(optarg, &endptr, 10);
				if (*endptr != 0) {
					error("Argument for -o has to be a number.\nTry -h to get help.\n");
					exit(2);
				}
				break;
			case 'l':
				resultLength = strtol(optarg, &endptr, 10);
				if (*endptr != 0) {
					error("Argument of -l has to be a number.\nTry -h to get help.\n");
					exit(2);
				}
				break;
			case 'c':
				charset = optarg;
				break;
			case 'e':
				decodeMode = false;
				break;
			case 'd':
				decodeMode = true;
				break;
			case 'v':
				verbose = true;
				break;
			case 'h':
				help(argv[0]);
				exit(0);
			default:
				error("Unknown option.\nTry -h to get help.\n");
				exit(2);
		}
	}

	if (optind >= argc) {
		error("Expected data after options.\nTry -h to get help.\n");
		exit(2);
	}

	verbose("charset:    %s\n", charset);
	verbose("length:     %d\n", resultLength);
	verbose("offset:     %d\n", offset);

	prepareCharset();

	settings_t settings = getSettings();

	if (decodeMode) {
		verbose("decode mode\n");
		const char* string = argv[optind];
		verbose("string:     %s\n", string);
		unsigned long long value = fromString(string, settings);
		value -= offset;
		printf("%llu\n", value);
	} else {
		unsigned long long value = strtoll(argv[optind], &endptr, 10);
		if (*endptr != 0) {
			error("Value has to be a number for encoding.\n");
			exit(2);
		}
		value += offset;
		if (value > settings.max) {
			error("value is too big!\nThe maximum value for the current settings is %llu.\n", settings.max - offset);
			exit(1);
		}
		verbose("encode mode\n");
		verbose("value:      %llu\n", value)
		char* string = malloc(resultLength);
		toString(string, value, settings);
		printf("%s\n", string);
		free(string);
	}
}
