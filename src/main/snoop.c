#include "snoop.h"

#include <stdio.h>
#include <string.h>

#include "serial_io.h"
#include "app_ping.h"
#include "utility.h"

void snoop_command(char* args)
{
	if (!args)
		{
			char* level = "";
			switch (snoop_level)
				{
				case SNOOP_LEVEL_NONE:
					level = "none";
					break;
				case SNOOP_LEVEL_PING:
					level = "ping";
					break;
				case SNOOP_LEVEL_CHAT:
					level = "chat";
					break;
				case SNOOP_LEVEL_ALL:
					level = "all";
					break;
				}

			// current... + level + null
			char buffer[21 + 4 + 1];
			sprintf(buffer, "Current snoop level: %s", level);
			serial_write_line(buffer);
		}
	else if (!strcmp(args, "none"))
		{
			snoop_level = SNOOP_LEVEL_NONE;
			serial_write_line("Snooping disabled");
		}
	else if (!strcmp(args, "ping"))
		{
			snoop_level = SNOOP_LEVEL_PING;
			serial_write_line("Now logging all pings");
		}
	else if (!strcmp(args, "chat"))
		{
			snoop_level = SNOOP_LEVEL_CHAT;
			serial_write_line("Now logging all chat messages");
		}
	else if (!strcmp(args, "all"))
		{
			snoop_level = SNOOP_LEVEL_ALL;
			serial_write_line("Now logging all packets");
		}
	else
		{
			// prefix + arg + null
			char buffer[18 + strlen(args) + 1];
			sprintf(buffer, "Invalid argument: %s", args);
			serial_write_line(buffer);
		}
}

void snoop_ping(const lownet_frame_t* frame)
{
	ping_packet_t* packet = (ping_packet_t*) frame->payload;
	lownet_time_t now = lownet_get_time();

	// id + action + id + newline + the... + time + newline + our... +
	// time + newline + null
	char buffer[4 + 11 + 4 + 1 + 12 + TIME_WIDTH + 1 + 10 + TIME_WIDTH + 1 + 1];
	int n;
	if (packet->origin != frame->destination)
		n = sprintf(buffer, "0x%x pinged 0x%x", frame->source, frame->destination);
	else
		n = sprintf(buffer, "0x%x replied to 0x%x", frame->source, frame->destination);

	n += sprintf(buffer + n, "\nTheir time: ");
	n += format_time(buffer + n, (lownet_time_t*) &packet->timestamp_out);
	n += sprintf(buffer + n, "\nOur time: ");
	n += format_time(buffer + n, &now);
	n += sprintf(buffer + n, "\n");

	serial_write_line(buffer);
}
void snoop_chat(const lownet_frame_t* frame)
{
	// origin id + says + message + to + destination id + null
	char buffer[4 + 7 + frame->length + 4 + 4 + 1];
	char msg[frame->length + 1];
	memcpy(msg, &frame->payload, frame->length);
	msg[frame->length] = '\0';
	sprintf(buffer, "0x%x says: %s to 0x%x", frame->source, msg, frame->destination);
	serial_write_line(buffer);
}

void snoop(const lownet_frame_t* frame)
{
	switch (frame->protocol)
		{
		case LOWNET_PROTOCOL_CHAT:
			snoop_chat(frame);
			return;
		case LOWNET_PROTOCOL_PING:
			snoop_ping(frame);
		}
}
