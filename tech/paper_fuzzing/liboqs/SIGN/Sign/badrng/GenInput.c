#include "../../../../utilities/liboqs_prng.h"
#include "../../../GenInput.h"
#include "../../../Call.h"
#include "../../../serialize.h"
#include <oqs/oqs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* 
 * The x will contain the message
 * The y will be the signature = Sign(sk, x) (fixed randomness)
 * The aux = (pk, sk)
 */ 

void GenInput(in_t *x, out_t *y, pp_t *pp, aux_t *aux, fmt_t *fmt)
{
    char prg_seed[48];
    memset(prg_seed, 0, 48);
    sprintf(prg_seed, "geninput");

#ifdef CHES_VERSION
    OQS_randombytes_custom_algorithm(&OQS_randombytes_nist_kat);
#else
    OQS_randombytes_switch_algorithm(OQS_RAND_alg_nist_kat);
#endif
    OQS_randombytes_nist_kat_init_256bit((uint8_t *)prg_seed, NULL);

    // x is the message

    // compute the total bytelength
    x->bytes = BufBytelen(fmt);
    // allocate the buffer to all 0s
    x->buf = (u8*) calloc(x->bytes, 1);
    // easier to see boundaries:

    // NOTE: y buffers are allocated inside Call, since they may require some specific
    //       knowledge of the format

    OQS_SIG *sig = NULL;
    const char *sig_name = OQS_SIG_alg_identifier(pp->alg_id);
    sig = OQS_SIG_new(sig_name);
    if (sig == NULL) {
        printf("%s was not enabled at compile-time.\n", sig_name);
        exit(1);
    }

    buf_list_init(aux, 1);

    u8 *public_key = malloc(sig->length_public_key);
    if (public_key == NULL) {
        fprintf(stderr, "ERROR: malloc failed!\n");
        OQS_SIG_free(sig);
        exit(1);
    }

    aux->list[0].bytes = sig->length_secret_key;
    u8 *secret_key = aux->list[0].buf = malloc(aux->list[0].bytes);
    if (secret_key == NULL) {
        fprintf(stderr, "ERROR: malloc failed!\n");
        OQS_SIG_free(sig);
        buf_list_free(aux);
        exit(1);
    }

    OQS_STATUS rc = OQS_SIG_keypair(sig, public_key, secret_key);

    if (rc != OQS_SUCCESS) {
        fprintf(stderr, "ERROR: keypair gen failed !\n");
    }

    // At this point, y is empty, x is a message of all 0, pp is the alg id, aux is the keypair, fmt is boh
    Call(y, x, pp, aux, fmt);

    free(public_key);
    OQS_SIG_free(sig);
}

int main(int argc, char **argv)
{
    // make sure an output file is specified
    bool using_afl = false;
    switch (argc)
    {
        case 3:
            printf("Assuming `make run_c` or `make run_python` is being used.\n");
            break;
        case 4:
            printf("Assuming `make run_afl` is being used.\n");
            using_afl = true;
            break;
        default:
            printf("Usage: %s out_fn.bin SIG_ID [mask_fn.bin]\n", argv[0]);
            return 0;
    }
    char *fn = argv[1];
    size_t sig_id = (size_t)atoi(argv[2]);
    assert(0 <= sig_id);
    assert(sig_id < OQS_SIG_algs_length);

    // prepare public parameters and format tuple for the GenInput call
    pp_t pp;
    pp.alg_id = sig_id; // 0 <= i < OQS_SIG_algs_length

    OQS_SIG *sig = NULL;
    const char *sig_name = OQS_SIG_alg_identifier(pp.alg_id);
    printf("%s\n", sig_name);
    sig = OQS_SIG_new(sig_name);
    if (sig == NULL) {
        printf("%s was not enabled at compile-time.\n", sig_name);
        exit(1);
    }

    fmt_t fmt;
    fmt.list_len = 1;
    fmt.list = (tuple_t *)malloc(fmt.list_len * sizeof(tuple_t));

    fmt.list[0].bitlen = 8; // one byte to be output as random byte
    fmt.list[0].lbl = DIFF;

    aux_t aux;
    in_t x;
    out_t y;
    GenInput(&x, &y, &pp, &aux, &fmt);

    // encode and save the content of (pp, fmt, x, x, y, expres) to disk (note the double x, one will be mauled)
    exp_res_t expres = EQ; // default value

    u8 *buf;
    u64 bytes;
    bytes = serialize(&buf, &pp, &aux, &fmt, &x, &x, &y, &expres);
    dump(fn, buf, bytes);

    if (using_afl)
    {
        // if we want to fuzz a mask, dump a 0 vector
        char *mask_fn = argv[3];
        u8 *mask = calloc(x.bytes, 1);
        dump(mask_fn, mask, x.bytes); // we dump 0 bytes of x.bytes bytelength
        free(mask);
    }

    #ifdef OQS_VERSION_NUMBER
    #if OQS_VERSION_NUMBER == 0x0201811fL
    size_t length_signature = sig->length_sig_overhead;
    #endif
    #else
    size_t length_signature = sig->length_signature;
    #endif

    buf_list_free(&aux);
    free(fmt.list);
    free(x.buf);
    free(buf);
    OQS_MEM_secure_free(y.buf, length_signature);
    OQS_SIG_free(sig);
    return 0;
}
