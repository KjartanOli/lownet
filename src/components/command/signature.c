#include "signature.h"

#include "utility.h"

bool signature_equal(const signature_t* a, const signature_t* b)
{
	return buffers_equal((const uint8_t*) a, (const uint8_t*) b, sizeof(signature_t));
}
