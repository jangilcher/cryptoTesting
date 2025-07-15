#include "GenInput.h"
#include "Call.h"
#include "serialize.h"
#include <stdlib.h>
#include <stdio.h>

void GenInput(in_t *x, out_t *y, pp_t *pp, fmt_t *fmt)
{
    // compute the total bytelength
    x->bytes = BufBytelen(fmt);
    // allocate the buffer to all 0s
    // printf("x->bytes = %"PRIu64"\n", x->bytes);
    // printf("Second Calloc\n");
    x->buf = (u8 *) calloc(x->bytes, 1); // option 1 in the text
    // easier to see boundaries:
    x->buf[0] = 0xFF;
    x->buf[x->bytes-1] = 0xFF;

    // evaluate H(0s) and store in the output buffer
    // NOTE: y buffers are allocated inside Call, since they may require some specific
    //       knowledge of the format

    // printf("Call\n");
    Call(y, x, pp, fmt);
}

int main(int argc, char **argv)
{
    // printf("Hello from GenInput\n");
    // make sure an output file is specified
    if (argc < 3 || argc > 4)
    {
        printf("Usage: %s out_fn.bin xbytelen [outbitlen]\n", argv[0]);
        return 0;
    }
    char *fn = argv[1];
    len_t xbytelen = atoll(argv[2]);
    len_t out_bitlen = 1024; // upper bound
    if (argc == 4) {
        out_bitlen = atoll(argv[3]) * 8;

    }
    // prepare public parameters and format tuple for the GenInput call
    pp_t pp;
    pp.out_bitlen = out_bitlen;
    fmt_t fmt;
    fmt.list_len = 3;
    // printf("First Calloc\n");
    fmt.list = (tuple_t *)calloc(fmt.list_len, sizeof(tuple_t));

    fmt.list[0].bitlen = 8; // left padding
    fmt.list[0].lbl = EQ;

    // bitlength of x
    fmt.list[1].bitlen = 8 * xbytelen; // bitlength of x
    // fmt.list[1].bitlen = 6;
    fmt.list[1].lbl = DIFF;

    fmt.list[2].bitlen = 8; // right padding
    fmt.list[2].lbl = EQ;

    in_t x;
    out_t y;
    GenInput(&x, &y, &pp, &fmt);

    // encode and save the content of (pp, fmt, x, x, y, expres) to disk (note the double x, one will be mauled)
    exp_res_t expres = EQ; // default value

    u8 *buf;
    u64 bytes;
    bytes = serialize(&buf, &pp, &fmt, &x, &x, &y, &expres);
    dump(fn, buf, bytes);

    free(fmt.list);
    free(x.buf);
    free(y.buf);
    free(buf);
    return 0;
}