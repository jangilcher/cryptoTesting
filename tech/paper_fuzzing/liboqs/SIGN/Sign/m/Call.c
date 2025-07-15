#include "../../../../utilities/liboqs_prng.h"
#include "../../../Call.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oqs/oqs.h>

void Call(out_t *out, in_t *in, pp_t *pp, aux_t *aux, fmt_t *fmt)
{
    char prg_seed[48];
    memset(prg_seed, 0, 48);
    sprintf(prg_seed, "call");

#ifdef CHES_VERSION
    OQS_randombytes_custom_algorithm(&OQS_randombytes_nist_kat);
#else
    OQS_randombytes_switch_algorithm(OQS_RAND_alg_nist_kat);
#endif
    OQS_randombytes_nist_kat_init_256bit((uint8_t *)prg_seed, NULL);

    OQS_SIG *sig = NULL;
    const char *sig_name = OQS_SIG_alg_identifier(pp->alg_id);
    sig = OQS_SIG_new(sig_name);
    if (sig == NULL) {
        fprintf(stderr, "%s was not enabled at compile-time.\n", sig_name);
        exit(1);
    }

    #ifdef OQS_VERSION_NUMBER
    #if OQS_VERSION_NUMBER == 0x0201811fL
    size_t length_signature = sig->length_sig_overhead;
    #endif
    #else
    size_t length_signature = sig->length_signature;
    #endif
    //aux->list[2].bytes = sizeof(size_t) + length_signature;

    out->bytes = length_signature + sizeof(size_t);
    out->buf = (u8 *)calloc(out->bytes, 1);

    size_t offset = bits_to_bytes(fmt->list[0].bitlen);
    size_t msg_len = bits_to_bytes(fmt->list[1].bitlen);

    size_t sign_len;
    OQS_STATUS rc = OQS_SIG_sign(sig, &(out->buf[sizeof(size_t)]), &sign_len, &(in->buf[offset]), msg_len, aux->list[1].buf);
    memcpy((size_t *) out->buf, &sign_len, sizeof(size_t));
    out->retval = rc;

    // Store in aux.list[2].buf the length
    memcpy((size_t *) aux->list[2].buf, &sign_len, sizeof(size_t));
    OQS_SIG_free(sig);
}
