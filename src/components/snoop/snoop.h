#ifndef SNOOP_H
#define SNOOP_H

#include <stddef.h>

#include "lownet.h"

#define SNOOP_LEVEL_NONE 0
#define SNOOP_LEVEL_PING 1 << 0
#define SNOOP_LEVEL_CHAT 1 << 2
#define SNOOP_LEVEL_ALL SNOOP_LEVEL_PING | SNOOP_LEVEL_CHAT

extern uint8_t snoop_level;

// Usage: snoop_command(LEVEL)
// Pre:   LEVEL is NULL, or one of: none, ping, chat, all
// Post:  If LEVEL is NULL the current snoop level has been written
//        to the serial port.  Otherwise the snoop level has been set
//        to the appropriate level.
void snoop_command(char* args);

// Usage: snoop(FRAME)
// Pre:   FRAME != NULL
// Post:  Information about FRAME has been written to the serial port.
void snoop(const lownet_frame_t* frame);
#endif
