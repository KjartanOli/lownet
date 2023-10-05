#include "mask.h"

#include <stdio.h>

#include "serial_io.h"
#include "utility.h"

void mask_command(char* args)
{
	mask_id = (uint8_t) hex_to_dec(args + 2);

	// Now cla... + id + null
	char buffer[20 + 4 + 1];
	sprintf(buffer, "Now claiming to be: 0x%x", mask_id);
	serial_write_line(buffer);
}

void unmask_command(char*)
{
	mask_id = MASK_UNMASKED;
}
