#include "crypt.h"

#include <stdlib.h>
#include <string.h>

#include <esp_log.h>
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
										(unsigned char*) &cipher->ivt,
										(unsigned char*) &cipher->frame,
										(unsigned char*) &plain->frame);
	esp_aes_free(&ctx);
}

void crypt_encrypt(const lownet_secure_frame_t* plain, lownet_secure_frame_t* cipher)
{
	const uint8_t* aes_key = lownet_get_key()->bytes;

	esp_aes_context ctx;

	esp_aes_init(&ctx);

	if (esp_aes_setkey(&ctx, aes_key, 256))
		{
			serial_write_line("Failed to set key!");
			return;
		}

	unsigned char ivt[LOWNET_IVT_SIZE];
	memcpy(ivt, &plain->ivt, LOWNET_IVT_SIZE);

	memcpy(&cipher->ivt, &plain->ivt, LOWNET_IVT_SIZE);
	esp_aes_crypt_cbc(&ctx,
										ESP_AES_ENCRYPT,
										MSG_LEN,
										(unsigned char*) ivt,
										(unsigned char*) &plain->frame,
										(unsigned char*) &cipher->frame);

	esp_aes_free(&ctx);
}

// Usage: crypt_command(KEY)
// Pre:   KEY is a valid AES key or NULL
// Post:  If key == NULL encryption has been disabled
//        Else KEY has been set as the encryption key to use for
//        lownet communication.
void crypt_setkey_command(char* args)
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

void crypt_test_command(char* str)
{
	if (!str)
		return;
	if (!lownet_get_key())
		{
			serial_write_line("No encryption key set!");
			return;
		}

	// Encrypts and then decrypts a string, can be used to sanity check your
	// implementation.
	lownet_secure_frame_t plain;
	lownet_secure_frame_t cipher;
	lownet_secure_frame_t back;

	memset(&plain, 0, sizeof(lownet_secure_frame_t));
	memset(&cipher, 0, sizeof(lownet_secure_frame_t));
	memset(&back, 0, sizeof(lownet_secure_frame_t));

	*((uint32_t*) plain.ivt) = 123456789;
	strcpy((char*) plain.frame.payload, str);

	crypt_encrypt(&plain, &cipher);
	crypt_decrypt(&cipher, &back);

	if (strlen((char*) back.frame.payload) != strlen(str)) {
		ESP_LOGE("APP", "Length violation");
	} else {
		serial_write_line((char*) back.frame.payload);
	}
}
