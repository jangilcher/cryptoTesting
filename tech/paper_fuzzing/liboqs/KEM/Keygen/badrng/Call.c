#include "../../../Call.h"
#include "../../../../utilities/liboqs_prng.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <oqs/oqs.h>

void Call(out_t *out, in_t *in, pp_t *pp, aux_t *aux, fmt_t *fmt)
{
    if (in->buf == NULL) {
        assert(0);
        // This branch is currently not executed, since otherwise
        // afl cannot verify that the "good sample" does not crash
        char prg_seed[48];
        memset(prg_seed, 0, 48);
        sprintf(prg_seed, "call");

    #ifdef CHES_VERSION
        OQS_randombytes_custom_algorithm(&OQS_randombytes_nist_kat);
    #else
        OQS_randombytes_switch_algorithm(OQS_RAND_alg_nist_kat);
    #endif
        OQS_randombytes_nist_kat_init_256bit((uint8_t *)prg_seed, NULL);

        // allocate the buffer to all 0s
        in->buf = (u8*) calloc(in->bytes, 1);
    } else {
        prng_global_init(in->buf, (uint64_t)in->bytes);
        OQS_randombytes_custom_algorithm(&prng_global_randombytes_system);
    }

    OQS_KEM *kem = NULL;
    const char *kem_name = OQS_KEM_alg_identifier(pp->alg_id);
    kem = OQS_KEM_new(kem_name);
    if (kem == NULL) {
        fprintf(stderr, "%s was not enabled at compile-time.\n", kem_name);
        exit(1);
    }

    out->bytes = kem->length_secret_key + kem->length_public_key;
    out->buf = (u8 *)calloc(out->bytes, 1);
    u8 *secret_key = out->buf;
    u8 *public_key = &out->buf[kem->length_secret_key];

    OQS_STATUS rc = OQS_KEM_keypair(kem, public_key, secret_key);
    out->retval = rc;

    OQS_KEM_free(kem);
}