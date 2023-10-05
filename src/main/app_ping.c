#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_ping.h"

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

	ping(dest);
}

void ping(uint8_t node) {
	lownet_frame_t frame;
	frame.source = lownet_get_device_id();
	frame.destination = node;
	frame.protocol = LOWNET_PROTOCOL_PING;
	frame.length = sizeof(ping_packet_t);
	((ping_packet_t*) frame.payload)->timestamp_out = lownet_get_time();
	((ping_packet_t*) frame.payload)->origin = lownet_get_device_id();

	lownet_send(&frame);
}


void ping_receive(const lownet_frame_t* frame) {
	if (frame->length < sizeof(ping_packet_t)) {
		// Malformed frame.  Discard.
		return;
	}

	if (frame->destination != lownet_get_device_id()
			&& frame->destination != LOWNET_BROADCAST_ADDRESS
			&& (mask_id != MASK_UNMASKED && frame->destination != mask_id))
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
			uint32_t t = (now.seconds * LOWNET_TIME_RESOLUTION + now.parts) - (packet->timestamp_out.seconds * LOWNET_TIME_RESOLUTION + packet->timestamp_out.parts);

			lownet_time_t rtt;
			rtt.seconds = t / LOWNET_TIME_RESOLUTION;
			rtt.parts = t % LOWNET_TIME_RESOLUTION;

			// reply from + id + rtt: + time + null
			char buffer[12 + 4 + 5 + TIME_WIDTH + 1];
			int n = 0;
			n += sprintf(buffer + n, "Reply from: 0x%x RTT: ", frame->source);
			n += format_time(buffer + n, &rtt);
			serial_write_line(buffer);
		}
	else
		{
			// Ping... + id (+ addre... + id) + null
			char buffer[11 + 4 + 16 + 4 + 1 + 1];
			int n = 0;
			n += sprintf(buffer, "Ping from: 0x%x", frame->source);
			if (frame->destination == mask_id)
				sprintf(buffer + n, " (Addressed to: 0x%x)", frame->destination);
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
