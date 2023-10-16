#include "crypt.h"

void crypt_decrypt(const lownet_secure_frame_t* cipher, lownet_secure_frame_t* plain)
{
	const uint8_t* aes_key = lownet_get_key()->bytes;

	// IMPLEMENT ME
}

void crypt_encrypt(const lownet_secure_frame_t* plain, lownet_secure_frame_t* cipher)
{
	const uint8_t* aes_key = lownet_get_key()->bytes;

	// IMPLEMENT ME
}

void crypt_command(char* args)
{
	
}
