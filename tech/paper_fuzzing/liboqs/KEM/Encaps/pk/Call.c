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

    OQS_KEM *kem = NULL;
    const char *kem_name = OQS_KEM_alg_identifier(pp->alg_id);
    kem = OQS_KEM_new(kem_name);
    if (kem == NULL) {
        fprintf(stderr, "%s was not enabled at compile-time.\n", kem_name);
        exit(1);
    }

    u8 *shared_secret_e = calloc(kem->length_shared_secret, 1);
    if (shared_secret_e == NULL) {
        fprintf(stderr, "ERROR: malloc failed!\n");
        OQS_KEM_free(kem);
        exit(1);
    }

    u8 *ciphertext = calloc(kem->length_ciphertext, 1);
    if (ciphertext == NULL) {
        fprintf(stderr, "ERROR: malloc failed!\n");
        free(shared_secret_e);
        OQS_KEM_free(kem);
        exit(1);
    }

    u64 offset = bits_to_bytes(fmt->list[0].bitlen);
    OQS_STATUS rv_encaps = OQS_KEM_encaps(kem, ciphertext, shared_secret_e, &(in->buf[offset]));
    if (rv_encaps != OQS_SUCCESS) {
        fprintf(stderr, "ERROR: OQS_KEM_encaps failed!\n");
        buf_list_free(aux);
        OQS_KEM_free(kem);
        exit(1);
    }

    u8 *shared_secret_f = calloc(kem->length_shared_secret, 1);
    if (shared_secret_f == NULL) {
        fprintf(stderr, "ERROR: malloc failed!\n");
        free(shared_secret_e);
        buf_list_free(aux);
        OQS_KEM_free(kem);
        exit(1);
    }

    u8 *secret_key = aux->list[0].buf;
    OQS_STATUS rv_decaps = OQS_KEM_decaps(kem, shared_secret_f, ciphertext, secret_key);
    int ss_eq = memcmp(shared_secret_e, shared_secret_f, kem->length_shared_secret);

    // in this version we save [[ss_e == ss_f]]
    // this one will capture either same ciphertext or different ciphertext encapsulating same key
    out->bytes = sizeof(int);
    out->buf = (u8 *)calloc(out->bytes, 1);
    memcpy(out->buf, &ss_eq, out->bytes);

    // // in this version we save the ciphertext
    // out->bytes = kem->length_ciphertext;
    // out->buf = (u8 *)calloc(out->bytes, 1);
    // memcpy(out->buf, ciphertext, out->bytes);

    out->retval = rv_decaps;
    free(shared_secret_e);
    free(shared_secret_f);
    free(ciphertext);
    OQS_KEM_free(kem);
}