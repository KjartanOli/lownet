#include "network.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "utility.h"
#include "serial_io.h"

network_node_t network[NETWORK_MAX_SIZE];
size_t network_size = 0;

// Usage: compare_time(LHS, RHS)
// Pre:   LSH != NULL, RHS != NULL
// Value: -1 if LSH is smaller than RHS
//         0 if LSH is equal to RHS
//         1 if LSH is greater than RHS
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

void network_command(char*)
{
	serial_write_line("Network:");
	for (size_t i = 0; i < network_size; ++i)
		{
			char buffer[1 + ID_WIDTH + 13 + TIME_WIDTH + 1];
			int n = 0;
			n += sprintf(buffer + n, "\t");
			n += format_id(buffer + n, network[i].id);
			n += sprintf(buffer + n, ": Last seen: ");
			format_time(buffer + n, &network[i].last_seen);
			serial_write_line(buffer);
		}
}

void network_register_node(uint8_t id)
{
	lownet_time_t now = lownet_get_time();
	/*
		Loop invariant:
		0 <= i < network_size <= NETWORK_MAX_SIZE
		forall x | 0 <= x < i : network[x].id != id
	 */
	for (size_t i = 0; i < network_size; ++i)
		if (network[i].id == id)
			{
				memcpy(&network[i].last_seen, &now, sizeof(lownet_time_t));
				return;
			}

	if (network_size == NETWORK_MAX_SIZE)
		{
			size_t oldest = 0;
			/*
				Loop invariant:
				0 < i < NETWORK_MAX_SIZE
				forall x | 0 <= x < i : network[oldest] < network[x]
			 */
			for (size_t i = 1; i < NETWORK_MAX_SIZE; ++i)
				if (compare_time(&network[i].last_seen, &network[oldest].last_seen) < 0)
					oldest = i;

			network[oldest].id = id;
			memcpy(&network[oldest].last_seen, &now, sizeof(lownet_time_t));
		}
	else
		{
			network[network_size].id = id;
			memcpy(&network[network_size].last_seen, &now, sizeof(lownet_time_t));
			++network_size;
		}
}
