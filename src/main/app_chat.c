#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <string.h>

#include "lownet.h"
#include "serial_io.h"
#include "utility.h"

#include "app_chat.h"

void chat_receive(const lownet_frame_t* frame) {
	if (!(frame->destination == lownet_get_device_id()
				|| frame->destination == 0xFF))
		return;

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
// Value: true if MESSAGE is a valid message, false otherwise
bool chat_valid_message(const char* message)
{
	size_t length = strlen(message);
	if (length > LOWNET_PAYLOAD_SIZE)
		return false;

	for (int i = 0; message[i]; i++)
		{
			if (!util_printable(message[i]))
				return false;
		}

	return true;
}

void chat_shout(const char* message) {
	if (!(message || chat_valid_message(message)))
		return;

	size_t length = strlen(message);

	lownet_frame_t frame;
	frame.source = lownet_get_device_id();
	frame.destination = 0xFF;
	frame.protocol = LOWNET_PROTOCOL_CHAT;
	frame.length = length;
	memcpy(&frame.payload, message, length);
	frame.payload[length] = '\0';
	lownet_send(&frame);
}

void chat_tell(const char* message, uint8_t destination) {
		if (!(message || chat_valid_message(message)))
			return;

	size_t length = strlen(message);

	lownet_frame_t frame;
	frame.source = lownet_get_device_id();
	frame.destination = destination;
	frame.protocol = LOWNET_PROTOCOL_CHAT;
	frame.length = length;
	memcpy(&frame.payload, message, length);
	frame.payload[length] = '\0';
	lownet_send(&frame);
}
