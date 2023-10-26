#ifndef COMMAND_H
#define COMMAND_H

#include "lownet.h"

// Usage: command_init()
// Pre:   None
// Post:  The command module has been initialised to receive commands
// Note:  This function should only be called once
void command_init();

// Usage: command_receive(FRAME)
// Pre:   FRAME is a command protocol frame
// Post:  FRAME has been processed according to the specification
void command_receive(const lownet_frame_t* frame);

#endif
