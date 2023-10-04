#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_ping.h"

#include "serial_io.h"

typedef struct __attribute__((__packed__))
{
	lownet_time_t timestamp_out;
	lownet_time_t timestamp_back;
	uint8_t origin;
} ping_packet_t;


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
		// Not intended for us
		return;

	if (((ping_packet_t*) frame->payload)->origin == lownet_get_device_id())
		{
			ping_packet_t* packet = ((ping_packet_t*) frame->payload);
			lownet_time_t rtt;
			rtt.seconds = packet->timestamp_back.seconds - packet->timestamp_out.seconds;
			rtt.parts = packet->timestamp_back.parts - packet->timestamp_out.parts;

			// reply from + id + rtt: + seconds + unit + part + suffix
			char buffer[12 + 4 + 5 + 11 + 2 + 3 + 4];
			sprintf(buffer, "Reply from: 0x%x RTT: %lus %u/256", frame->source, rtt.seconds, rtt.parts);
			serial_write_line(buffer);
		}
	else {
		lownet_frame_t reply;
		reply.source = lownet_get_device_id();
		reply.destination = frame->source;
		reply.protocol = frame->protocol;
		memcpy(&reply.payload, frame->payload, sizeof(ping_packet_t));
		((ping_packet_t*) &reply.payload)->timestamp_back = lownet_get_time();
		lownet_send(&reply);
	}
}
