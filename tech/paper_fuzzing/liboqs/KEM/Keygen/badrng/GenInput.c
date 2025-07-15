#include "../../../GenInput.h"
#include "../../../Call.h"
#include "../../../serialize.h"
#include <oqs/oqs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/*
pk, sk <- gen
c, ss <- encaps
ss' <- decaps

aux = pk, sk
in = c
out = ss

fmt = format(in)  (what to change or not)
*/

void GenInput(in_t *x, out_t *y, pp_t *pp, aux_t *aux, fmt_t *fmt)
{
    // compute the total bytelength
    x->bytes = BufBytelen(fmt);
    // allocate the buffer to all 0s
    x->buf = (u8 *)calloc(1, 1);

    buf_list_init(aux, 0);

    Call(y, x, pp, aux, fmt);
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
            printf("Usage: %s out_fn.bin KEM_ID [mask_fn.bin]\n", argv[0]);
            return 0;
    }
    char *fn = argv[1];
    size_t kem_id = (size_t)atoi(argv[2]);
    assert(0 <= kem_id);
    assert(kem_id < OQS_KEM_algs_length);

    // prepare public parameters and format tuple for the GenInput call
    pp_t pp;
    pp.alg_id = kem_id; // 0 <= i < OQS_KEM_algs_length

    OQS_KEM *kem = NULL;
    const char *kem_name = OQS_KEM_alg_identifier(pp.alg_id);
    printf("%s\n", kem_name);
    kem = OQS_KEM_new(kem_name);
    if (kem == NULL) {
        printf("%s was not enabled at compile-time.\n", kem_name);
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
        // Dump a mask of all 0
        char *mask_fn = argv[3];
        u8 *mask = calloc(x.bytes, 1);
        dump(mask_fn, mask, x.bytes); // we dump 0 bytes of x.bytes bytelength
        free(mask);
    }

    buf_list_free(&aux);
    free(fmt.list);
    free(x.buf);
    free(buf);
    OQS_MEM_secure_free(y.buf, kem->length_public_key + kem->length_secret_key);
    OQS_KEM_free(kem);
    return 0;
}
