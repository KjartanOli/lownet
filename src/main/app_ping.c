#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_ping.h"

#include "serial_io.h"
#include "snoop.h"

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
			&& frame->destination != 0xFF)
		{
			if (snoop_level & SNOOP_LEVEL_PING)
				snoop(frame);
			else
				// Not intended for us
				return;
		}

	if (((ping_packet_t*) frame->payload)->origin == lownet_get_device_id())
		{
			lownet_time_t now = lownet_get_time();
			ping_packet_t* packet = ((ping_packet_t*) frame->payload);
			uint32_t rtt = (now.seconds * LOWNET_TIME_RESOLUTION + now.parts) - (packet->timestamp_out.seconds * LOWNET_TIME_RESOLUTION + packet->timestamp_out.parts);

			uint32_t seconds = rtt / LOWNET_TIME_RESOLUTION;
			uint8_t parts = rtt % LOWNET_TIME_RESOLUTION;

			// reply from + id + rtt: + seconds + unit + part + suffix + null
			char buffer[12 + 4 + 5 + 11 + 2 + 3 + 4 + 1];
			sprintf(buffer, "Reply from: 0x%x RTT: %lus %u/256", frame->source, seconds, parts);
			serial_write_line(buffer);
		}
	else
		{
			// Ping... + id + null
			char buffer[11 + 4 + 1];
			sprintf(buffer, "Ping from: 0x%x", frame->source);
			serial_write_line(buffer);

			lownet_frame_t reply;
			reply.source = lownet_get_device_id();
			reply.destination = frame->source;
			reply.protocol = frame->protocol;
			reply.length = sizeof(ping_packet_t);
			memcpy(&reply.payload, frame->payload, sizeof(ping_packet_t));
			((ping_packet_t*) &reply.payload)->timestamp_back = lownet_get_time();
			lownet_send(&reply);
		}
}
