#include "../../../../utilities/liboqs_prng.h"
#include "../../../Call.h"
#include "../../../../utilities/liboqs_prng.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oqs/oqs.h>

void Call(out_t *out, in_t *in, pp_t *pp, aux_t *aux, fmt_t *fmt)
{
    prng_global_init(in->buf, (uint64_t)in->bytes);
    OQS_randombytes_custom_algorithm(&prng_global_randombytes_system);

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

    out->bytes = length_signature + sizeof(size_t);
    out->buf = (u8 *)calloc(out->bytes, 1);

    size_t msg_len = 1;
    u8 *message = calloc(msg_len, 1); // sign the message 0x00

    size_t sign_len;
    OQS_STATUS rc = OQS_SIG_sign(sig, &(out->buf[sizeof(size_t)]), &sign_len, message, msg_len, aux->list[0].buf);
    memcpy((size_t *) out->buf, &sign_len, sizeof(size_t));
    out->retval = rc;

    // NOTE: may want to check reval in Match instead
    free(message);
    OQS_SIG_free(sig);
}
