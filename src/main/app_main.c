// CSTDLIB includes.
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// LowNet includes.
#include "lownet.h"

#include "serial_io.h"
#include "utility.h"
#include "commands.h"
#include "snoop.h"
#include "mask.h"
#include "network.h"

#include "app_chat.h"
#include "app_ping.h"

const char* ERROR_OVERRUN = "ERROR // INPUT OVERRUN";
const char* ERROR_UNKNOWN = "ERROR // PROCESSING FAILURE";

const char* ERROR_COMMAND = "Command error";
const char* ERROR_ARGUMENT = "Argument error";

void help_command(char*);

const command_t commands[] = {
	{"shout",   "/shout MSG                   Broadcast a message.", shout_command},
	{"tell",    "/tell ID MSG or @ID MSG      Send a message to a specific node", tell_command},
	{"ping",    "/ping ID                     Check if a node is online", ping_command},
	{"date",    "/date                        Print the current time", date_command},
	{"id",      "/id                          Print your ID", id_command},
	{"snoop",   "/snoop [none|ping|chat|all]  Set the level of snooping on other's communications", snoop_command},
	{"mask",    "/mask ID                     Pretend to be node ID", mask_command},
	{"unmask",  "/unmask                      Stop id masking", unmask_command},
	{"network", "/network                     Print information about the network", network_command},
	{"help",    "/help                        Print this help", help_command}
};

const size_t NUM_COMMANDS = sizeof commands / sizeof(command_t);
#define FIND_COMMAND(_command) (find_command(_command, commands, NUM_COMMANDS))

void help_command(char*)
{
	for (size_t i = 0; i < NUM_COMMANDS; ++i)
		serial_write_line(commands[i].description);
	serial_write_line("Any input not preceded by a '/' or '@' will be treated as a broadcast message.");
}

uint8_t snoop_level = SNOOP_LEVEL_NONE;
uint8_t mask_id = MASK_UNMASKED;


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
						command_fun_t command = FIND_COMMAND(name);
						if (!command)
							{
								char buffer[17 + strlen(name) + 1];
								sprintf(buffer, "Invalid command: %s", name);
								serial_write_line(buffer);
								continue;
							}
						char* args = strtok(NULL, "\n");
						command(args);
					}
				else if (msg_in[0] == '@')
					{
						FIND_COMMAND("tell")(msg_in + 1);
					}
				else
					{
						// Default, chat broadcast message.
						FIND_COMMAND("shout")(msg_in);
					}
			}
		}
}
