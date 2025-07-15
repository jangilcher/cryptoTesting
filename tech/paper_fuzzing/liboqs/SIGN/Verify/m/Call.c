#include "../../../../utilities/liboqs_prng.h"
#include "../../../Call.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oqs/oqs.h>

void Call(out_t *out, in_t *in, pp_t *pp, aux_t *aux, fmt_t *fmt)
{
    #ifdef OQS_VERSION_NUMBER
    #if OQS_VERSION_NUMBER == 0x0201811fL
    // NOT SUPPORTED, old liboqs uses sign_open rather than verify, and the internals change per scheme
    fprintf(stderr, "liboqs version not supported.\n");
    exit(1);
    #endif
    #endif

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

    out->bytes = sizeof(OQS_STATUS);
    out->buf = (u8 *)malloc(out->bytes);

    // NOTE: Passing in->buf is not enough! need to use fmt_t, and point to the initial byte of intended input
    //       For this reason, we instead define `offset` and then hash &(in->buf[offset]).
    //       Making this primitive-dependent eases the need for super complicated abstract handling of the format, etc
    u64 offset = bits_to_bytes(fmt->list[0].bitlen);

    u8 *signature = &(aux->list[2].buf[sizeof(size_t)]);
    size_t signature_len = *(size_t *)aux->list[2].buf;
    OQS_STATUS rc = OQS_SIG_verify(sig, &(in->buf[offset]), bits_to_bytes(fmt->list[1].bitlen), signature, signature_len, aux->list[0].buf);

    memcpy((OQS_STATUS*) out->buf, &rc, out->bytes);
    out->retval = rc;
    // out->retval = OQS_SUCCESS;  // this is the only place where LibOQS uses retval as meaning something other than "function returned"
                                // so we store that as out->buf, and set out->retval as OQS_SUCCESS, since function returned
    OQS_SIG_free(sig);
}
