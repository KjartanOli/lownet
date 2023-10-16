#ifndef CRYPT_H
#define CRYPT_H

#include "lownet.h"

void crypt_decrypt(const lownet_secure_frame_t* cipher, lownet_secure_frame_t* plain);
void crypt_encrypt(const lownet_secure_frame_t* plain, lownet_secure_frame_t* cipher);

void crypt_command(char* args);
#endif
