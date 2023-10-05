#ifndef NETWORK_H
#define NETWORK_H

#include "lownet.h"

#define NETWORK_MAX_SIZE 5

typedef struct
{
	uint8_t id;
	lownet_time_t last_seen;
} network_node_t;

// Usage: network_register_node(ID)
// Pre:   ID is a node id
// Post: The node identified by ID has been registered as part of the
//       network.
void network_register_node(uint8_t id);

// Usage: network_command(NULL)
// Pre:   None, this command takes no arguments
// Post:  A overview of the known network has been written to the serial port.
void network_command(char*);
#endif
