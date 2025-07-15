#pragma once
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <oqs/oqs.h>
#ifdef CHES_VERSION
    #include <oqs/rand_nist.h>
#endif
typedef struct
{
    uint8_t *buffer;
    uint64_t bytes_read;
    uint64_t output_bytelen;
} prng_state_t;

extern prng_state_t prng_global_state;

void prng_global_init(uint8_t *output_buffer, uint64_t output_bytelen);
void prng_global_next();
void prng_global_randombytes_system(uint8_t *random_array, size_t bytes_to_read);
void prng_global_example();

#ifdef OQS_VERSION_NUMBER
#if OQS_VERSION_NUMBER == 0x0201811fL

// NOTE: for old_liboqs we should call OQS_randombytes_nist_kat_init(_,_,256) instead
OQS_API void OQS_randombytes_nist_kat_init_256bit(const uint8_t *entropy_input, const uint8_t *personalization_string);

#endif
#endif