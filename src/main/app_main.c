// CSTDLIB includes.
#include <stdio.h>
#include <string.h>

// LowNet includes.
#include "lownet.h"

#include "serial_io.h"
#include "utility.h"

#include "app_chat.c"
#include "app_ping.c"

const char* ERROR_OVERRUN = "ERROR // INPUT OVERRUN";
const char* ERROR_UNKNOWN = "ERROR // PROCESSING FAILURE";

const char* ERROR_COMMAND = "Command error";
const char* ERROR_ARGUMENT = "Argument error";

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

void date()
{
	lownet_time_t time = lownet_get_time();
	if (time.seconds == 0 && time.parts == 0)
		{
			serial_write_line("Network time is not available.\n");
			return;
		}

	// seconds + period + parts + description
	char buffer[11 + 1 + 3 + 32];
	sprintf(buffer, "%lu.%u sec since the course started.\n", time.seconds, time.parts);
	serial_write_line(buffer);
}

// Usage: ctoi(C)
// Pre:   C is a valid hexadecimal character
// Value: The decimal value of C as a hexadecimal character or 255 if
//        C is not a valid hexadecimal character
uint8_t ctoi(char c)
{
	if ('0' <= c && c <= '9')
		return c - '0';
	else if ('a' <= c && c <= 'f')
		return 10 + c - 'a';
	else if ('A' <= c && c <= 'F')
		return 10 + c - 'A';
	else
		return 255;
}

// Usage: parse_id(STR, DEST)
// Pre:   STR != NULL,
//        3 <= strlen(STR) <= 4,
//        STR is a valid hexadecimal number,
//        DEST != NULL
// Value: true if STR was successfully parsed into a valid id
// Post: The value pointed to by DEST has been set to the id parsed
//       from STR
bool parse_id(char* str, uint8_t* dest)
{
	if (!str)
		return false;

	size_t length = strlen(str);
	if (length < 3 || length > 4 || str[0] != '0' || str[1] != 'x')
		return false;

	if (length == 3)
		{
			uint8_t t = ctoi(str[2]);
			if (t == 255)
				return false;
			*dest = t;
		}

	uint8_t t = ctoi(str[2]);
	if (t == 255)
		return false;
	*dest = t * 16;
	t = ctoi(str[3]);
	if (t == 255)
		return false;
	*dest += t;

	return true;
}

void app_main(void)
{
	char msg_in[MSG_BUFFER_LENGTH];
	char msg_out[MSG_BUFFER_LENGTH];

	// Initialize the serial services.
	init_serial_service();

	// Initialize the LowNet services.
	lownet_init(app_frame_dispatch);

	while (true) {
		memset(msg_in, 0, MSG_BUFFER_LENGTH);
		memset(msg_out, 0, MSG_BUFFER_LENGTH);

		if (!serial_read_line(msg_in)) {
			// Quick & dirty input parse.  Presume input length > 0.
			if (msg_in[0] == '/') {
				char* command = strtok(msg_in + 1, " ");
				char* argument = strtok(NULL, " ");

				if (!strcmp(command, "ping"))
					{
						if (!argument)
							{
								serial_write_line("A node id must be provided\n");
								continue;
							}

						uint8_t dest;
						if (!parse_id(argument, &dest))
							{
								serial_write_line("Invalid node id\n");
								continue;
							}

						ping(dest);
					}
				else if (!strcmp(command, "date"))
					date();
			} else if (msg_in[0] == '@') {
				char* dest = strtok(msg_in + 1, " ");
				char* message = strtok(NULL, "\n");

				uint8_t d;
				if (!parse_id(dest, &d))
					{
						serial_write_line("Invalid node id\n");
						continue;
					}

				if (!message)
					{
						serial_write_line("A message must be provided\n");
						continue;
					}

				chat_tell(message, d);
			} else {
				// Default, chat broadcast message.
				chat_shout(msg_in);
			}
		}
	}
}
