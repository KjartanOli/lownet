#include "commands.h"

#include <string.h>
#include <stdio.h>

#include "utility.h"
#include "serial_io.h"
#include "snoop.h"
#include "mask.h"

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
	if (mask_id == MASK_UNMASKED)
		{
			// id + null
			char buffer[4 + 1];
			sprintf(buffer, "0x%x", lownet_get_device_id());
			serial_write_line(buffer);
		}
	else
		{
			// id + (mask... + id + ) + null
			char buffer[4 + 12 + 4 + 1 + 1];
			sprintf(buffer, "0x%x (Masked as 0x%x)", lownet_get_device_id(), mask_id);
			serial_write_line(buffer);
		}
}

void date_command(char*)
{
	lownet_time_t time = lownet_get_time();
	if (time.seconds == 0 && time.parts == 0)
		{
			serial_write_line("Network time is not available.");
			return;
		}

	// seconds + unit + part + description + null
	char buffer[TIME_WIDTH + 1];
	int n = format_time(buffer, &time);
	sprintf(buffer + n, " since the course started");
	serial_write_line(buffer);
}
