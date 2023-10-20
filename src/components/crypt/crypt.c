#include "crypt.h"

#include <stdlib.h>
#include <string.h>

#include <aes/esp_aes.h>

#include "serial_io.h"
#include "lownet.h"

#define MSG_LEN LOWNET_FRAME_SIZE + LOWNET_CRYPTPAD_SIZE

void crypt_decrypt(const lownet_secure_frame_t* cipher, lownet_secure_frame_t* plain)
{
	const uint8_t* aes_key = lownet_get_key()->bytes;

	esp_aes_context ctx;

	esp_aes_init(&ctx);

	if (esp_aes_setkey(&ctx, aes_key, 256))
		{
			serial_write_line("Failed to set key!");
			return;
		}

	esp_aes_crypt_cbc(&ctx,
										ESP_AES_DECRYPT,
										MSG_LEN,
										&cipher->ivt,
										&cipher->frame,
										&plain->frame);
	esp_aes_free(&ctx);
}

void crypt_encrypt(const lownet_secure_frame_t* plain, lownet_secure_frame_t* cipher)
{
	memcpy(&plain->ivt, &cipher->ivt, LOWNET_IVT_SIZE);

	const uint8_t* aes_key = lownet_get_key()->bytes;

	esp_aes_context ctx;

	esp_aes_init(&ctx);

	if (esp_aes_setkey(&ctx, aes_key, 256))
		{
			serial_write_line("Failed to set key!");
			return;
		}

	esp_aes_crypt_cbc(&ctx,
										ESP_AES_ENCRYPT,
										MSG_LEN,
										&plain->ivt,
										&plain->frame,
										&cipher->frame);

	esp_aes_free(&ctx);
}

// Usage: crypt_command(KEY)
// Pre:   KEY is a valid AES key or NULL
// Post:  If key == NULL encryption has been disabled
//        Else KEY has been set as the encryption key to use for
//        lownet communication.
void crypt_command(char* args)
{
	if (!args)
		{
			lownet_set_key(NULL);
			return;
		}

	lownet_key_t key;
	if (!strcmp(args, "0"))
		{
			key = lownet_keystore_read(0);
			serial_write_line("Using stored key 0");
		}
	else if (!strcmp(args, "1"))
		{
			key = lownet_keystore_read(1);
			serial_write_line("Using stored key 1");
		}
	else
		{
			key.size = LOWNET_KEY_SIZE_AES;
			key.bytes = calloc(key.size, sizeof(uint8_t));

			size_t length = strlen(args);
			memcpy(key.bytes, args, length);
		}

	lownet_set_key(&key);
}
