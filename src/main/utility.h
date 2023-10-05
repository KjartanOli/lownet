#ifndef GUARD_UTILITY_H
#define GUARD_UTILITY_H

#include "lownet.h"

int util_printable(char c);

uint32_t hex_to_dec(const char* hex_digits);

#define TIME_WIDTH 21
// Usage: format_time(BUFFER, TIME)
// Pre:   BUFFER != NULL, TIME != NULL
//        sizeof BUFFER >= TIME_WIDTH
// Post:  TIME has been formatted into buffer
// Value: The number of characters written to BUFFER
int format_time(char* buffer, lownet_time_t* time);

#endif
