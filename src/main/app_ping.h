#ifndef GUARD_APP_PING_H
#define GUARD_APP_PING_H

#include <stdint.h>

#include "lownet.h"

// Usage: ping_command(ID)
// Pre:   ID is a valid node id.
// Post:  A ping has been sent to the node identified by ID.
void ping_command(char* args);


void ping(uint8_t node);

void ping_receive(const lownet_frame_t* frame);

typedef struct __attribute__((__packed__))
{
	lownet_time_t timestamp_out;
	lownet_time_t timestamp_back;
	uint8_t origin;
} ping_packet_t;

#endif
