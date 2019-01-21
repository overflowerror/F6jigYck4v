#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "sfuid.h"

const sfuid_settings_t sfuid_default_settings = {
	.length = 10,
	.charset = "a-zA-Z0-9",
	.offset = 5
};

sfuid_settings_t settings;
bool settingsValid = false;

struct {
	uint64_t max;
	uint64_t prime;
	char* charset;
	int inverseCharset[(1 << 7)];
} properties = {
	.charset = NULL,
	.inverseCharset = {-1}	
};


typedef __uint128_t uint128_t;
typedef __int128_t int128_t;

#ifdef DEBUG
	#define verbose(...) printf(__VA_ARGS__);
#else
	#define verbose(...)
#endif 

int convertToString(char* string, uint64_t number) {
	int charsetLength = strlen(properties.charset);

	int length = 0;
	do {
		string[length++] = properties.charset[number % charsetLength];
		number /= charsetLength;
	} while(number > 0);

	for(; length < settings.length; length++) {
		string[length] = properties.charset[0];
	}

	for(int i = 0; i < length / 2; i++) {
		string[i] ^= string[length - i - 1];
		string[length - i - 1] ^= string[i];
		string[i] ^= string[length - i - 1];
	}

	string[length] = '\0';

	return SFUID_ERROR_NONE;
}

int convertFromString(const char* string, uint64_t* result) {
	int stringLength = strlen(string);
	int charsetLength = strlen(properties.charset);

	*result = 0;

	for (int i = 0; i < stringLength; i++) {
		int tmp = properties.inverseCharset[(int) string[i]];
		if (tmp < 0) {
			return SFUID_ERROR_PARSING;
		}
		uint64_t val = ((uint64_t) pow(charsetLength, stringLength - i - 1));
		*result += ((uint64_t) tmp) * val;
	}

	return SFUID_ERROR_NONE;
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

int decode(uint64_t* value, uint64_t prime, uint64_t maxValue) {
	/* 
	 * (x * p) mod m = v
	 * (x * p) == v   (mod m)
	 * Def: i = multiplicative inverse of p
	 * => x = v * i
	 * => p * i == i  (mod m)
	 */

	uint128_t tmp = *value;

	uint64_t inverse = multiplicativeInverse(prime, maxValue);

	//verbose("inverse:    %llu\n", inverse);

	tmp *= inverse;
	tmp &= (maxValue);
	*value = tmp;

	return SFUID_ERROR_NONE;
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

	int size = strlen(settings.charset);
	const char* tmp;

	for(int i = 0; i < ((sizeof shortcuts) / (sizeof shortcuts[0])); i++) {
		tmp = settings.charset;
		while((tmp = strstr(tmp, shortcuts[i].needle)) != NULL) {
			tmp++;
			size += strlen(shortcuts[i].replacement) - strlen(shortcuts[i].needle);
		}
	}

	char* inflatedCharset = malloc(size + 1);
	strcpy(inflatedCharset, settings.charset);

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

	if (properties.charset != NULL)
		free(properties.charset);

	properties.charset = inflatedCharset;
}

void prepareInverseCharset() {
	for(int i = 0; i < 127; i++) {
		properties.inverseCharset[i] = -1;
	}
	for(int i = 0; i < strlen(properties.charset); i++) {
		properties.inverseCharset[(int) properties.charset[i]] = i;
	}
}

int setProperties() {
	prepareCharset();
	prepareInverseCharset();

	verbose("charset length: %d\n", strlen(properties.charset));
	double maxValueDouble = pow(strlen(properties.charset), settings.length);
	uint64_t maxValue;
	int maxBits; 

	if (maxValueDouble >= pow(2, 65)) {
		verbose("Max value %.0lf > 2^64, caping to 64 bits.\n", maxValueDouble);
		maxBits = 64;
	} else {
		maxValue = (uint64_t) maxValueDouble;
		verbose("theoer max: %llu\n", maxValue);
		maxBits = findMax(maxValue);
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

	properties.max = maxValue;
	properties.prime = prime;

	return SFUID_ERROR_NONE;
}

int checkCharset() {
	int tmp = strlen(properties.charset);
	if (tmp <= 0)
		return SFUID_ERROR_CHARSET;
	for(int i = 0; i < tmp; i++) {
		if (properties.charset[i] < 0 || properties.charset[i] > 127)
			return SFUID_ERROR_CHARSET;
		for(int j = i+1; j < tmp; j++) {
			if (properties.charset[i] == properties.charset[j])
				return SFUID_ERROR_CHARSET;
		}
	}
	return SFUID_ERROR_NONE;
}

int sfuid_init(sfuid_settings_t _settings) {
	settings = _settings;
	settingsValid = false;

	if (settings.length == 0) {
		return SFUID_ERROR_LENGTH;
	}

	int tmp = setProperties();
	if (tmp != SFUID_ERROR_NONE)
		return tmp;

	tmp = checkCharset();
	if (tmp != SFUID_ERROR_NONE)
		return tmp;

	settingsValid = true;
	
	return SFUID_ERROR_NONE;
}

int sfuid_encode(uint64_t value, char* string) {
	if (!settingsValid)
		return SFUID_ERROR_SETTINGS_INVALID;

	#ifdef BENCHMARK
		struct timespec before, after;
		clock_gettime(CLOCK_MONOTONIC, &before);
	#endif

	value += settings.offset;

	if (value > properties.max) 
		return SFUID_ERROR_VALUE;

	value = encode(value, properties.prime, properties.max);
	int tmp = convertToString(string, value);
	if (tmp != SFUID_ERROR_NONE)
		return tmp;

	#ifdef BENCHMARK
		clock_gettime(CLOCK_MONOTONIC, &after);
		unsigned long long diff = (int64_t)(after.tv_sec - before.tv_sec) * (int64_t) 1000000000UL + (int64_t)(after.tv_nsec - before.tv_nsec);
		fprintf(stderr, "Time for conversion: %llu ns\n", diff);
	#endif


	return SFUID_ERROR_NONE;
}

int sfuid_decode(const char* string, uint64_t* value) {
	if (!settingsValid)
		return SFUID_ERROR_SETTINGS_INVALID;

	#ifdef BENCHMARK
		struct timespec before, after;
		clock_gettime(CLOCK_MONOTONIC, &before);
	#endif


	int tmp = convertFromString(string, value);
	if (tmp != SFUID_ERROR_NONE)
		return tmp;

	tmp = decode(value, properties.prime, properties.max);
	if (tmp != SFUID_ERROR_NONE)
		return tmp;

	*value -= settings.offset;

	#ifdef BENCHMARK
		clock_gettime(CLOCK_MONOTONIC, &after);
		unsigned long long diff = (int64_t)(after.tv_sec - before.tv_sec) * (int64_t) 1000000000UL + (int64_t)(after.tv_nsec - before.tv_nsec);
		fprintf(stderr, "Time for conversion: %llu ns\n", diff);
	#endif

	return SFUID_ERROR_NONE;
}

const char* errors[] = {
	"No error.",
	"Length parameter not valid.",
	"Charset parameter not valid.",
	"Error while parsing input.",
	"Settings not valid.",
	"Value not valid."
};

const char* sfuid_error(int error) {
	if (error < 0 || error >= ((sizeof errors) / sizeof errors[0])) {
		return "Unknown error.";
	}
	return errors[error];
}
