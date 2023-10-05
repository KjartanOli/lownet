#include "commands.h"

#include <string.h>
#include <stdio.h>

#include "utility.h"
#include "serial_io.h"

#include "app_ping.h"
#include "app_chat.h"

command_fun_t find_command(const char* command, const command_t* commands, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		if (!strcmp(command, commands[i].name))
			return commands[i].fun;

	return NULL;
}

void id_command(char*)
{
	// id + null
	char buffer[5];
	sprintf(buffer, "0x%x", lownet_get_device_id());
	serial_write_line(buffer);
}

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

void shout_command(char* args)
{
	chat_shout(args);
}

void tell_command(char* args)
{
	char* dest = strtok(args, " ");
	char* message = strtok(NULL, "\n");

	uint8_t d = (uint8_t) hex_to_dec(dest + 2);
	if (d == 0)
		{
			serial_write_line("Invalid node id\n");
			return;
		}

	if (!message)
		{
			serial_write_line("A message must be provided\n");
			return;
		}

	chat_tell(message, d);
}

void date_command(char*)
{
	lownet_time_t time = lownet_get_time();
	if (time.seconds == 0 && time.parts == 0)
		{
			serial_write_line("Network time is not available.\n");
			return;
		}

	// seconds + unit + part + description + null
	char buffer[11 + 2 + 3 + 24 + 0];
	sprintf(buffer, "%lus %u/256 since the course started", time.seconds, time.parts);
	serial_write_line(buffer);
}
