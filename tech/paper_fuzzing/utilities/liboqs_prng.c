#include "liboqs_prng.h"
#include "bufutils.h"
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

prng_state_t prng_global_state;

void prng_global_init(uint8_t *randomness_buffer, uint64_t randomness_bytelen)
{
    prng_global_state.buffer = randomness_buffer;
    prng_global_state.output_bytelen = randomness_bytelen;
    prng_global_state.bytes_read = 0;
}

void prng_global_next()
{
    prng_global_state.bytes_read = 0;
}

void prng_global_randombytes_system_read_a_byte(uint8_t *random_array)
{
    // if (prng_global_state.bytes_read < prng_global_state.output_bytelen)
    // {
    //     *random_array = prng_global_state.buffer[prng_global_state.bytes_read];
    // }
    // else
    // {
    //     *random_array = 'A'; // using 'A' rather than 0 makes it easier to spot by eye, debug purposes
    // }
    *random_array = prng_global_state.buffer[prng_global_state.bytes_read % prng_global_state.output_bytelen];
    prng_global_state.bytes_read++;
}

void prng_global_randombytes_system(uint8_t *random_array, size_t bytes_to_read)
{
    for (size_t i = 0; i < bytes_to_read; i++)
    {
        prng_global_randombytes_system_read_a_byte(random_array + i);
    }
}

#ifdef OQS_VERSION_NUMBER
#if OQS_VERSION_NUMBER == 0x0201811fL

// NOTE: for old_liboqs we should call OQS_randombytes_nist_kat_init(_,_,256) instead
OQS_API void OQS_randombytes_nist_kat_init_256bit(const uint8_t *entropy_input, const uint8_t *personalization_string) {
    OQS_randombytes_nist_kat_init(entropy_input, personalization_string, 256);
}

#endif
#endif

#if defined(UNIT_TESTS)

#include <oqs/oqs.h>

void prng_global_example()
{
    uint64_t i;

    // // prepare a "random" tape to be output by the fake prng
    // uint8_t *random_tape;
    // int entropy_len = 10;
    // random_tape = (uint8_t *)malloc(entropy_len);
    // for (size_t i = 0; i < entropy_len; i++)
    // {
    //     random_tape[i] = 0 + 255 * (int)(i % 2);
    // }
    // prng_global_init(random_tape, (uint64_t)entropy_len);

    uint8_t random_char = 'A';
    uint64_t entropy_len = 1;

    // either one can set up a custom prng
    prng_global_init(&random_char, (uint64_t)entropy_len);
    OQS_randombytes_custom_algorithm(&prng_global_randombytes_system);

    // or one can setup the prng NIST uses for their KATs. 
    // This takes a random tape that should be 48 bytes long, and "personalization_string",
    // also 48 bytes long, XOR's them together and uses that as seed for a AES256 CTR DBRG
    // OQS_randombytes_switch_algorithm(OQS_RAND_alg_nist_kat);
    // uint8_t personalization_string[48];
    // for (size_t i = 0; i < 48; i++)
    // {
    //     personalization_string[i] = 0;
    // }
    // personalization_string[0] = 1;
    // OQS_randombytes_nist_kat_init_256bit(random_tape, personalization_string);

    uint8_t *out_buffer;
    out_buffer = (uint8_t *)malloc(entropy_len * 2);

    // initial read with all zeros
    OQS_randombytes(out_buffer, entropy_len * 2);
    print_buffer_bin(out_buffer, entropy_len * 2);

    free(out_buffer);
    // free(random_tape);
}

int main ()
{
    prng_global_example();
}

#endif