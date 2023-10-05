#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <string.h>

#include "lownet.h"
#include "serial_io.h"
#include "utility.h"
#include "snoop.h"

#include "app_chat.h"

void chat_receive(const lownet_frame_t* frame) {
	if (!(frame->destination == lownet_get_device_id()
				|| frame->destination == 0xFF))
		{
			if (snoop_level & SNOOP_LEVEL_CHAT)
				return snoop(frame);
			else
				return;
		}

	char msg[frame->length + 1];
	memcpy(msg, &frame->payload, frame->length);
	msg[frame->length] = '\0';

	// id + max(len("shouts"), len("says")) + message + null
	char buffer[4 + 9 + frame->length + 1];
	sprintf(buffer, "0x%x %s: %s",
					frame->source,
					(frame->destination == lownet_get_device_id()) ? "says" : "shouts",
					msg);
	serial_write_line(buffer);
}

// Usage: chat_valid_message(MESSAGE)
// Pre:   MESSAGE != NULL
// Value: strlen(MESSAGE) if MESSAGE is a valid message, 0 otherwise
size_t chat_valid_message(const char* message)
{
	size_t i = 0;
	for (; message[i]; i++)
		{
			if (!util_printable(message[i]))
				return 0;
		}

	if (i > LOWNET_PAYLOAD_SIZE)
		return 0;


	return i;
}

void chat_shout(const char* message) {
	chat_tell(message, 0xFF);
}

void chat_tell(const char* message, uint8_t destination) {
	size_t length = 0;
	if (!(message && (length = chat_valid_message(message))))
		return;

	lownet_frame_t frame;
	frame.source = lownet_get_device_id();
	frame.destination = destination;
	frame.protocol = LOWNET_PROTOCOL_CHAT;
	frame.length = length;
	memcpy(&frame.payload, message, length);
	frame.payload[length] = '\0';
	lownet_send(&frame);
}
