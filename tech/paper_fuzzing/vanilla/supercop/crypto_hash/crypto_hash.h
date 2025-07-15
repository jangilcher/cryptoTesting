#ifndef CRYPTO_HASH_H
#define CRYPTO_HASH_H
#include "api.h"
int crypto_hash(
	unsigned char *out,
	const unsigned char *in,
	unsigned long long inlen
	);

#define crypto_hash_BYTES CRYPTO_BYTES
#define CRYPTO_NAMESPACE(s) s
#endif

