#include "command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial_io.h"

#include <mbedtls/sha256.h>
#include <mbedtls/rsa.h>
#include <mbedtls/pk.h>

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
	WAIT_SIG,
	WAIT_SIG1,
	WAIT_SIG2,
} state_t;

typedef enum
{
	UNSIGNED = 0b00,
	SIGNED = 0b01,
	SIG1 = 0b10,
	SIG2 = 0b11,
} frame_type_t;

typedef uint8_t hash_t[CMD_HASH_SIZE];
typedef uint8_t signature_t[CMD_BLOCK_SIZE];

static_assert(sizeof(hash_t) == CMD_HASH_SIZE, "hash_t size");
static_assert(sizeof(signature_t) == CMD_BLOCK_SIZE, "signature_t size");

struct {
	state_t state;
	uint64_t last_valid;
	cmd_packet_t current_cmd;
	hash_t hash;
	signature_t signature;
	mbedtls_pk_context pk;
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
	memset(&state.current_cmd, 0, sizeof(cmd_packet_t));
	memset(&state.hash, 0, sizeof(hash_t));
	memset(&state.signature, 0, sizeof(signature_t));
}

// Usage: compare_hash(HASH)
// Pre:   HASH is a buffer of size CMD_HASH_SIZE
// Value: true if HASH is equal to the hash of the frame currently
//        being processed, false otherwise
bool compare_hash(const hash_t hash)
{
	return memcmp(hash, &state.hash, sizeof(hash_t)) == 0;
}

void command_init()
{
	command_ready_next();
	state.last_valid = 0;
	mbedtls_pk_init(&state.pk);

	if (mbedtls_pk_parse_public_key(&state.pk,
																	(const unsigned char*) lownet_get_signing_key(),
																	strlen(lownet_get_signing_key()) + 1))
		serial_write_line("failed to init public key");
}

// Usage: verify_signature()
// Pre:   A full signature has been received
// Value: true if the received signature matches the current command
bool verify_signature()
{
	signature_t signature;
	mbedtls_rsa_public(mbedtls_pk_rsa(state.pk),
										 state.signature,
										 signature);

	signature_t expected;
	memset(expected, 0, 220);
	memset(expected + 220, 1, 4);
	memcpy(expected + 220 + 4, state.hash, sizeof(hash_t));

	return memcmp(expected, signature, sizeof(signature_t)) == 0;
}

// Usage: command_execute()
// Pre:   The signature of the current command has been verified
// Post:  The current command has been executed
void command_execute()
{
	serial_write_line("executing");
}

// Usage: signature_received()
// Pre:   A full signature for the current command frame has been
//        received
// Post: The signature of the current command frame has been verified
//       and if correct the command has been executed
void signature_received()
{
	/* print_rsa(state.signature); */
	if (!verify_signature())
		{
			// Invalid signature, discard the command.
			command_ready_next();
			return;
		}

	command_execute();
}

void handle_command_frame(const lownet_frame_t* frame)
{
	const cmd_packet_t* command = (const cmd_packet_t*) &frame->payload;
	if (command->sequence < state.last_valid)
		return;

	// Discard command in processing to handle this new one.
	// TODO: Allow multiple commands at the same time.
	command_ready_next();

	if (mbedtls_sha256((const unsigned char*) frame, sizeof(lownet_frame_t), state.hash, 0))
		{
			// Something went wrong hashing the frame, discard it.
			command_ready_next();
			return;
		}

	memcpy(&state.current_cmd, command, sizeof(cmd_packet_t));
	state.state = WAIT_SIG;

	char buffer[255];
	sprintf(buffer, "Command {\n\tType: %d\n\tSequence: %llx,\n\tCommand: %d\n}\n", get_frame_type(frame), command->sequence, command->type);
	serial_write_line(buffer);
}

void handle_signature_part1(const cmd_signature_t* signature)
{
	memcpy(state.signature, signature->sig_part, sizeof signature->sig_part);
	if (state.state == WAIT_SIG)
		{
			state.state = WAIT_SIG2;
			return;
		}

	if (state.state == WAIT_SIG1)
		signature_received();
}

void handle_signature_part2(const cmd_signature_t* signature)
{
	memcpy(state.signature + (sizeof(signature_t) / 2), signature->sig_part, sizeof signature->sig_part);
	if (state.state == WAIT_SIG)
		{
			state.state = WAIT_SIG1;
			return;
		}

	if (state.state == WAIT_SIG2)
		signature_received();
}

// Usage: handle_signature_frame(FRAME)
// Pre:   get_frame_type(FRAME) = SIG1 or SIG2
// Post:  FRAME has been processed
void handle_signature_frame(const lownet_frame_t* frame)
{
	frame_type_t type = get_frame_type(frame);
	const cmd_signature_t* signature = (const cmd_signature_t*) &frame->payload;

	// If the msg hash does not match the current command this is a
	// signature for a different command.  Discard it.
	if (!compare_hash(signature->hash_msg))
		{
			serial_write_line("hash mismatch");
			return;
		}


	switch (type)
		{
		case SIG1:
			handle_signature_part1(signature);
			return;
		case SIG2:
			handle_signature_part2(signature);
		default:
			return;
		}
}

void command_receive(const lownet_frame_t* frame)
{
	frame_type_t type = get_frame_type(frame);

	switch (type)
		{
		case UNSIGNED:
			return;

		case SIGNED:
			handle_command_frame(frame);
			return;

		case SIG1:
		case SIG2:
			handle_signature_frame(frame);
			return;
		}
}
