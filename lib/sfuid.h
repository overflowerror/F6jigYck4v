#ifndef SFUID_H
#define SFUID_H

#include <stdint.h>

#define SFUID_ERROR_NONE 0
#define SFUID_ERROR_LENGTH 1
#define SFUID_ERROR_CHARSET 2
#define SFUID_ERROR_PARSING 3
#define SFUID_ERROR_SETTINGS_INVALID 4
#define SFUID_ERROR_VALUE 5


typedef struct {
	unsigned int length;
	const char* charset;
	uint64_t offset;
} sfuid_settings_t;

extern const sfuid_settings_t sfuid_default_settings;

int sfuid_init(sfuid_settings_t settings);
int sfuid_encode(uint64_t value, char* string);
int sfuid_decode(const char* string, uint64_t* value);

const char* sfuid_error(int error);

#endif
