#ifndef MASK_H
#define MASK_H

#include <stdint.h>

#define MASK_UNMASKED 0

extern uint8_t mask_id;

// Usage: mask_command(ID)
// Pre:  ID is a node id.
// Post: All future frams will be sent with ID as source.
void mask_command(char* args);

// Usage: unmask_command(NULL)
// Pre:   None, this command takes no arguments.
// Post:  Any masking applied by mask_command has been removed.
void unmask_command(char*);

#endif
