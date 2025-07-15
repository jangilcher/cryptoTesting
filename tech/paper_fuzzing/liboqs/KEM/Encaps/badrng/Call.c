#include "../../../../utilities/liboqs_prng.h"
#include "../../../Call.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oqs/oqs.h>

void Call(out_t *out, in_t *in, pp_t *pp, aux_t *aux, fmt_t *fmt)
{
    prng_global_init(in->buf, (uint64_t)in->bytes);
    OQS_randombytes_custom_algorithm(&prng_global_randombytes_system);

    OQS_KEM *kem = NULL;
    const char *kem_name = OQS_KEM_alg_identifier(pp->alg_id);
    kem = OQS_KEM_new(kem_name);
    if (kem == NULL) {
        fprintf(stderr, "%s was not enabled at compile-time.\n", kem_name);
        exit(1);
    }

    out->bytes = kem->length_shared_secret + kem->length_ciphertext;
    out->buf = (u8 *)calloc(out->bytes, 1);
    if (out->buf == NULL) {
        fprintf(stderr, "ERROR: malloc failed!\n");
        OQS_KEM_free(kem);
        exit(1);
    }
    u8 *shared_secret_e = out->buf;
    u8 *ciphertext = &(out->buf[kem->length_shared_secret]);

    u8 *public_key = aux->list[0].buf;

    OQS_STATUS rv = OQS_KEM_encaps(kem, ciphertext, shared_secret_e, public_key);
    out->retval = rv;
    OQS_KEM_free(kem);
}