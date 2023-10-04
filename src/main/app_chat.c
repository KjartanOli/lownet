
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
	if (frame->destination == lownet_get_device_id()) {
		// This is a tell message, just for us!

		// id + " says: " + null terminator
		char buffer[4 + 7 + 1];
		sprintf(buffer, "0x%x says: ", frame->source);
		serial_write_line(buffer);
	} else if (frame->destination == 0xFF) {
		// id + " shouts: " + null terminator
		char buffer[4 + 9 + 1];
		sprintf(buffer, "0x%x shouts: ", frame->source);
		serial_write_line(buffer);
	}

	char buffer[frame->length + 1];
	memcpy(&buffer, &frame->payload, frame->length);
	buffer[frame->length] = '\0';
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
