#include "command.h"

#include <stdio.h>
#include "serial_io.h"

#define FRAME_TYPE_UNSIGNED 0b00
#define FRAME_TYPE_SIGNED 0b01
#define FRAME_TYPE_SIG1 0b10
#define FRAME_TYPE_SIG2 0b11

// Usage: get_frame_type(FRAME)
// Pre:   FRAME is a valid lownet frame
// Value: The type of the frame
uint8_t get_frame_type(const lownet_frame_t* frame)
{
	return (frame->protocol & 0b11000000) >> 6;
}

typedef struct __attribute__((__packed__))
{
	uint64_t sequence;
	uint8_t command;
	uint8_t reserved[3];
	char data[180];
} command_t;

typedef enum
{
	LISTENING,
	SIGNATURE_1,
	SIGNATURE_2
} state_t;

struct {
	state_t state;
} state;

void command_receive(const lownet_frame_t* frame)
{
	const command_t* command = (const command_t*) &frame->payload;

	char buffer[255];
	sprintf(buffer, "Command {\n\tType: %d\n\tSequence: %llx,\n\tCommand: %d\n}\n", get_frame_type(frame), command->sequence, command->command);
	serial_write_line(buffer);
}
