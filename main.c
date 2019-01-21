#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

typedef __uint128_t uint128_t;
typedef __int128_t int128_t;

const char* charset = "a-zA-Z0-9";
int inverseCharset[(1 << 7)] = {-1};
unsigned int resultLength = 10;
unsigned int offset = 5;
bool verbose = false;

#define verbose(...) if (verbose) printf(__VA_ARGS__);
#define error(...) fprintf(stderr, __VA_ARGS__)

int convertToString(char* string, uint64_t number) {
	int charsetLength = strlen(charset);

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

uint64_t convertFromString(const char* string) {
	uint64_t result = 0;

	int stringLength = strlen(string);
	int charsetLength = strlen(charset);

	for (int i = 0; i < stringLength; i++) {
		bool found = false;
		int tmp = inverseCharset[string[i]];
		if (tmp < 0) {
			error("Parsing error.\nUnexpected character '%c'.\n", string[i]);
			exit(3);
		}
		uint64_t val = ((uint64_t) pow(charsetLength, stringLength - i - 1));
		result += ((uint64_t) tmp) * val;
	}

	return result;
}

uint64_t primelist[] = {
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

int findMax(uint64_t theoreticalMax) {
	int result = 0;
	uint64_t tmp = 0;
	while(true) {
		if (tmp >= theoreticalMax)
			break;
		tmp = 1ll << ++result;
	}
	return result - 1;
}

uint64_t findPrime(int maxBit) {
	int primelistLength = (sizeof primelist) / (sizeof (long long));
	if (maxBit >= primelistLength)
		return primelist[primelistLength - 1];

	return primelist[maxBit - 1];
}

uint64_t encode(uint64_t value, uint64_t prime, uint64_t maxValue) {
	uint128_t result = value;

	result *= prime;

	result &= (maxValue);

	return (uint64_t) result;
}

uint64_t multiplicativeInverse(uint64_t prime, uint64_t maxValue) {
	/*
	 * I didn't write this. 
	 * I got this from https://rosettacode.org/wiki/Modular_inverse#C 
	 * and adopted it to use signed 128 bit data types (something else 
	 * wouldn't work, because we need a signed bit and 64 bits for the 
	 * value).
	 */
	int128_t a = prime;
	// we need the modulo value and not the maxium
	int128_t b = maxValue;
	b += 1;

	int128_t b0 = b, t, q;
	int128_t x0 = 0, x1 = 1;
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

uint64_t decode(uint64_t value, uint64_t prime, uint64_t maxValue) {
	/* 
	 * (x * p) mod m = v
	 * (x * p) == v   (mod m)
	 * Def: i = multiplicative inverse of p
	 * => x = v * i
	 * => p * i == i  (mod m)
	 */

	uint128_t tmp = value;

	uint64_t inverse = multiplicativeInverse(prime, maxValue);

	//verbose("inverse:    %llu\n", inverse);

	tmp *= inverse;
	tmp &= (maxValue);

	return (uint64_t) tmp;
}

typedef struct settings {
	uint64_t max;
	uint64_t prime;
} settings_t;

settings_t getSettings() {
	verbose("charset length: %d\n", strlen(charset));
	double maxValueDouble = pow(strlen(charset), resultLength);
	uint64_t maxValue;
	int maxBits; 

	if (maxValueDouble >= pow(2, 65)) {
		verbose("Max value %.0lf > 2^64, caping to 64 bits.\n", maxValueDouble);
		maxBits = 64;
	} else {
		maxValue = (uint64_t) maxValueDouble;
		verbose("theoer max: %llu\n", maxValue);
		maxBits = findMax(maxValue);

		if (maxBits <= 0) {
			error("Length or charset is too big. This should not have happend.\n");
			exit(3);
		}
	}

	if (maxBits == 64) {
		/* 
		 * we want to avoid undefined behaviour.
		 * so instead of using an underflow we'll 
		 * make this one explicit.
		 */
		maxValue = UINT64_MAX;

	} else {
		maxValue = 1ll << maxBits;
		maxValue--;
	}

	verbose("used bits:  %d\n", maxBits);
	verbose("max:        %llu\n", maxValue);
	verbose("usage:      %.1f%\n", (100.0 * maxValue / maxValueDouble));

	uint64_t prime = findPrime(maxBits);

	verbose("prime:      %llu\n", prime);

	return (settings_t) {.max = maxValue, .prime = prime};
}

void toString(char* string, uint64_t value, settings_t settings) {
	#ifdef BENCHMARK
		struct timespec before, after;
		clock_gettime(CLOCK_MONOTONIC, &before);
	#endif

	value = encode(value, settings.prime, settings.max);
	verbose("encoded:    %llu\n", value);
	convertToString(string, value);

	#ifdef BENCHMARK
		clock_gettime(CLOCK_MONOTONIC, &after);

		uint64_t diff = (int64_t)(after.tv_sec - before.tv_sec) * (int64_t) 1000000000UL + (int64_t)(after.tv_nsec - before.tv_nsec);
		fprintf(stderr, "Time for conversion: %llu ns\n", diff);
	#endif
}

uint64_t fromString(const char* string, settings_t settings) {
	#ifdef BENCHMARK
		struct timespec before, after;
		clock_gettime(CLOCK_MONOTONIC, &before);
	#endif

	uint64_t result = convertFromString(string);
	verbose("encoded:    %llu\n", result);
	result = decode(result, settings.prime, settings.max);

	#ifdef BENCHMARK
		clock_gettime(CLOCK_MONOTONIC, &after);

		uint64_t diff = (int64_t)(after.tv_sec - before.tv_sec) * (int64_t) 1000000000UL + (int64_t)(after.tv_nsec - before.tv_nsec);
		fprintf(stderr, "Time for conversion: %llu ns\n", diff);
	#endif


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
	const char* tmp;

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
				// we declared tmp as const char* but now we are using it for char
				first = (char*) tmp;
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

void prepareInverseCharset() {
	for(int i = 0; i < strlen(charset); i++) {
		inverseCharset[charset[i]] = i;
	}
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

	while((opt = getopt(argc, argv, "o:c:l:devhy")) != -1) {
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
			case 'y':
				charset = "0-9A-Za-z-_";
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

	verbose("charset:    %s\n", charset);
	verbose("length:     %d\n", resultLength);
	verbose("offset:     %d\n", offset);

	prepareCharset();

	settings_t settings = getSettings();

	if (decodeMode) {
		prepareInverseCharset();
		verbose("decode mode\n");
		const char* string = argv[optind];
		verbose("string:     %s\n", string);
		uint64_t value = fromString(string, settings);
		value -= offset;
		printf("%llu\n", value);
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
