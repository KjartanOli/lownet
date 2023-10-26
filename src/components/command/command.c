#include "command.h"

#include <stdio.h>
#include "serial_io.h"

#define CMD_HASH_SIZE 32
#define CMD_BLOCK_SIZE 256

typedef struct __attribute__((__packed__))
{
	uint64_t sequence;
	uint8_t type;
	uint8_t reserved[3];
	uint8_t contents[180];
} cmd_packet_t;

typedef struct __attribute__((__packed__))
{
	uint8_t hash_key[CMD_HASH_SIZE];
	uint8_t hash_msg[CMD_HASH_SIZE];
	uint8_t sig_part[CMD_BLOCK_SIZE / 2];
} cmd_signature_t;

typedef enum
{
	LISTENING,
	SIGNATURE_1,
	SIGNATURE_2
} state_t;

typedef enum
{
	UNSIGNED = 0b00,
	SIGNED = 0b01,
	SIG1 = 0b10,
	SIG2 = 0b11,
} frame_type_t;

struct {
	state_t state;
	uint64_t last_valid;
	char hash[CMD_HASH_SIZE];
} state;

// Usage: get_frame_type(FRAME)
// Pre:   FRAME is a valid lownet frame
// Value: The type of the frame
frame_type_t get_frame_type(const lownet_frame_t* frame)
{
	return (frame->protocol & 0b11000000) >> 6;
}

// Usage: command_ready_next()
// Pre:   Any command frame currently being processed has either
//        completed processing or been discarded
// Post: The command module is ready to receive another command packet
void command_ready_next()
{
	state.state = LISTENING;
	memset(&state.hash, 0, CMD_HASH_SIZE);
}

void command_init()
{
	command_ready_next();
	state.last_valid = 0;
}

void command_receive(const lownet_frame_t* frame)
{
	const cmd_packet_t* command = (const cmd_packet_t*) &frame->payload;

	char buffer[255];
	sprintf(buffer, "Command {\n\tType: %d\n\tSequence: %llx,\n\tCommand: %d\n}\n", get_frame_type(frame), command->sequence, command->type);
	serial_write_line(buffer);
}
