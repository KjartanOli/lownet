#ifndef COMMANDS_H
#define COMMANDS_H

#include <stddef.h>

/*
  A command consists of a name, a description, and a function which
  accepts a char* ARGS.  The value of ARGS and its interpretation is
  function defined.
 */

typedef void (*command_fun_t)(char* args);
typedef struct
{
	char* name;
	char* description;
	command_fun_t fun;
} command_t;

// Usage: find_command(COMMAND, COMMANDS, N)
// Pre:   COMMAND is the name of a command in COMMANDS.
//        COMMANDS is an array of command_t objects.
//        N is the length of the COMMANDS.
// Value: A pointer to the function that corresponds to the command COMMAND.
command_fun_t find_command (const char* command, const command_t* commands, size_t n);

// DEFINITION: A node id is a string of the form 0xXX where XX is a
// valid hexadecimal number.

// Usage: id_command(NULL)
// Pre:   None, this command takes no arguments.
// Post:  The id of the node has been written to the serial port.
void id_command(char* args);

// Usage: ping_command(ID)
// Pre:   ID is a valid node id.
// Post:  A ping has been sent to the node identified by ID.
void ping_command(char* args);

// Usage: shout_command(MSG)
// Pre:   MSG != NULL
// Post:  MSG as been broadcast over the network.
void shout_command(char* args);

// Usage: tell_command(ARGS)
// Pre:   ARGS is a string of the form 'ID MSG'
//        where ID is a node id number and MSG is a non
//        empty string.  ID and MSG must be separated by a single space.
// Post:  MSG has been sent to the node identified by ID.
void tell_command(char* args);

// Usage: date_command(NULL)
// Pre:   None, this command takes no arguments.
// Post:  The network time has been written to the serial port.
void date_command(char* args);

#endif
