// CSTDLIB includes.
#include <stdio.h>
#include <string.h>

// LowNet includes.
#include "lownet.h"

#include "serial_io.h"
#include "utility.h"
#include "commands.h"

#include "app_chat.c"
#include "app_ping.c"

const char* ERROR_OVERRUN = "ERROR // INPUT OVERRUN";
const char* ERROR_UNKNOWN = "ERROR // PROCESSING FAILURE";

const char* ERROR_COMMAND = "Command error";
const char* ERROR_ARGUMENT = "Argument error";

void help_command(char*);

const command_t commands[] = {
	{"shout", "/shout MSG               Broadcast a message.", shout_command},
	{"tell",  "/tell ID MSG or @ID MSG  Send a message to a specific node", tell_command},
	{"ping",  "/ping ID                 Check if a node is online", ping_command},
	{"date",  "/date                    Print the current time", date_command},
	{"id",    "/id                      Print your ID", id_command},
	{"help",  "/help                    Print this help", help_command}
};

const size_t NUM_COMMANDS = sizeof commands / sizeof(command_t);

void help_command(char*)
{
	for (size_t i = 0; i < NUM_COMMANDS; ++i)
		serial_write_line(commands[i].description);
	serial_write_line("Any input not preceded by a '/' or '@' will be treated as a broadcast message.");
}

void app_frame_dispatch(const lownet_frame_t* frame) {
	switch(frame->protocol) {
		case LOWNET_PROTOCOL_CHAT:
			chat_receive(frame);
			break;

		case LOWNET_PROTOCOL_PING:
			ping_receive(frame);
			break;
	}
}

void app_main(void)
{
	char msg_in[MSG_BUFFER_LENGTH];
	char msg_out[MSG_BUFFER_LENGTH];


	// Initialize the serial services.
	init_serial_service();

	// Initialize the LowNet services.
	lownet_init(app_frame_dispatch);

	while (true)
		{
			memset(msg_in, 0, MSG_BUFFER_LENGTH);
			memset(msg_out, 0, MSG_BUFFER_LENGTH);

			if (!serial_read_line(msg_in)) {
				// Quick & dirty input parse.
				if (msg_in[0] == 0) continue;
				if (msg_in[0] == '/')
					{
						char* name = strtok(msg_in + 1, " ");
						command_fun_t command = find_command(name, commands, (sizeof commands / sizeof(command_t)));
						if (!command)
							{
								serial_write_line("Invalid command:");
								serial_write_line(name);
								continue;
							}
						char* args = strtok(NULL, " ");
						command(args);
					}
				else if (msg_in[0] == '@')
					{
						command_fun_t command = find_command("tell", commands, (sizeof commands / sizeof(command_t)));
						command(msg_in + 1);
					}
				else
					{
						// Default, chat broadcast message.
						chat_shout(msg_in);
					}
			}
		}
}
