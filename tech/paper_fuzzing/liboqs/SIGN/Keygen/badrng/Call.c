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

    OQS_SIG *sig = NULL;
    const char *sig_name = OQS_SIG_alg_identifier(pp->alg_id);
    sig = OQS_SIG_new(sig_name);
    if (sig == NULL) {
        fprintf(stderr, "%s was not enabled at compile-time.\n", sig_name);
        exit(1);
    }

    out->bytes = sig->length_secret_key + sig->length_public_key;
    out->buf = (u8 *)calloc(out->bytes, 1);
    u8 *secret_key = out->buf;
    u8 *public_key = &out->buf[sig->length_secret_key];

    OQS_STATUS rc = OQS_SIG_keypair(sig, public_key, secret_key);
    out->retval = rc;

    OQS_SIG_free(sig);
}