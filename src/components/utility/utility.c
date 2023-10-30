#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "utility.h"

int util_printable(char c) {
	return ((c >= ' ' && c < 127)
		?	1
		:	0
	);
}

uint8_t min(uint8_t a, uint8_t b)
{
	return (a <= b) ? a : b;
}

int compare_time(const lownet_time_t* lhs, const lownet_time_t* rhs)
{
	if (lhs->seconds < rhs->seconds)
		return -1;
	else if (lhs->seconds > rhs->seconds)
		return 1;
	else
		{
			if (lhs->parts < rhs->parts)
				return -1;
			else if (lhs->parts > rhs->parts)
				return 1;
			else
				return 0;
		}
}

uint32_t hex_to_dec(const char* hex_digits) {
	const char* map = "0123456789abcdef";
	uint32_t acc = 0x00000000;

	for (int i = 0; i < strlen(hex_digits); ++i) {
		uint32_t addend = 0x10; // Too large for single digit, sentinel value.
		for (int j = 0; j < 16; ++j) {
			if (tolower(hex_digits[i]) == map[j]) {
				addend = j;
				break;
			}
		}
		if (addend > 0x0F) {
			// Invalid digit.
			return 0;
		}
		acc = (acc << 4) + addend;
	}
	return acc;
}

int format_time(char* buffer, lownet_time_t* time)
{
	return sprintf(buffer, "%lu.%lus", time->seconds, ((uint32_t)time->parts * 1000) / 256);
}

int format_id(char* buffer, uint8_t id)
{
	return sprintf(buffer, "0x%x", id);
}
