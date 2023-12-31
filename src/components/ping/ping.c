#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ping.h"

#include "serial_io.h"
#include "utility.h"
#include "snoop.h"
#include "mask.h"

void ping_command(char* args)
{
	if (!args)
		{
			serial_write_line("A node id must be provided\n");
			return;
		}

	uint8_t dest = (uint8_t) hex_to_dec(args + 2);
	if (dest == 0)
		{
			serial_write_line("Invalid node id\n");
			return;
		}

	ping(dest, NULL, 0);
}

void ping(uint8_t node, const uint8_t* payload, uint8_t length)
{
	lownet_frame_t frame;
	frame.source = (mask_id ? mask_id : lownet_get_device_id());
	frame.destination = node;
	frame.protocol = LOWNET_PROTOCOL_PING;
	frame.length = sizeof(ping_packet_t);
	((ping_packet_t*) frame.payload)->timestamp_out = lownet_get_time();
	((ping_packet_t*) frame.payload)->origin = (mask_id ? mask_id : lownet_get_device_id());

	if (payload)
		{
			memcpy(frame.payload + frame.length,
						 payload,
						 min(LOWNET_PAYLOAD_SIZE - sizeof(ping_packet_t), length));
			frame.length += min(LOWNET_PAYLOAD_SIZE - sizeof(ping_packet_t), length);
		}

	lownet_send(&frame);
}


void ping_receive(const lownet_frame_t* frame) {
	if (frame->length < sizeof(ping_packet_t)) {
		// Malformed frame.  Discard.
		return;
	}

	if (frame->destination != lownet_get_device_id()
			&& frame->destination != LOWNET_BROADCAST_ADDRESS
			&& frame->destination != mask_id)
		{
			if (snoop_level & SNOOP_LEVEL_PING)
				snoop(frame);
			else
				// Not intended for us
				return;
		}

	if (((ping_packet_t*) frame->payload)->origin == (mask_id ? mask_id : lownet_get_device_id()))
		{
			lownet_time_t now = lownet_get_time();
			ping_packet_t* packet = ((ping_packet_t*) frame->payload);

			lownet_time_t rtt = time_diff(&packet->timestamp_out, &now);

			// reply from + id + rtt: + time + null
			char buffer[12 + ID_WIDTH + 6 + TIME_WIDTH + 1];
			int n = 0;
			n += sprintf(buffer + n, "Reply from: ");
			n += format_id(buffer + n, frame->source);
			n += sprintf(buffer + n, " RTT: ");
			n += format_time(buffer + n, &rtt);
			serial_write_line(buffer);
		}
	else
		{
			// Ping... + id (+ addre... + id) + null
			char buffer[11 + ID_WIDTH + 16 + ID_WIDTH + 1 + 1];
			int n = 0;
			n += sprintf(buffer + n, "Ping from: ");
			n += format_id(buffer + n, frame->source);
			if (frame->destination == mask_id)
				{
					n += sprintf(buffer + n, " (Addressed to: ");
					n += format_id(buffer + n, frame->destination);
					n += sprintf(buffer + n, ")");
				}
			serial_write_line(buffer);

			lownet_frame_t reply;
			// make sure we don't unmask ourselves by replying with the id
			// the packet is addressed to.
			reply.source = frame->destination;
			reply.destination = frame->source;
			reply.protocol = frame->protocol;
			reply.length = sizeof(ping_packet_t);
			memcpy(&reply.payload, frame->payload, sizeof(ping_packet_t));
			((ping_packet_t*) &reply.payload)->timestamp_back = lownet_get_time();
			lownet_send(&reply);
		}
}
