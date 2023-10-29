#ifndef GUARD_UTILITY_H
#define GUARD_UTILITY_H

#include "lownet.h"

int util_printable(char c);

// Usage: min(A, B)
// Pre:   None, other than those imposed by the type system
// Value: The smaller of A and B
uint8_t min(uint8_t a, uint8_t b);

uint32_t hex_to_dec(const char* hex_digits);

// uint32 + '.' + uint32 + 's'
#define TIME_WIDTH (11 + 1 + 11 + 1)

// Usage: format_time(BUFFER, TIME)
// Pre:   BUFFER != NULL, TIME != NULL
//        sizeof BUFFER >= TIME_WIDTH
// Post:  TIME has been formatted into buffer
// Value: The number of characters written to BUFFER
int format_time(char* buffer, lownet_time_t* time);

#define ID_WIDTH 4
// Usage: format_id(BUFFER, ID)
// Pre:   BUFFER != NULL, sizeof BUFFER >= ID_WIDTH
// Post:  ID has been formatted into buffer
// Value: The number of characters written to BUFFER
int format_id(char* buffer, uint8_t id);
#endif
