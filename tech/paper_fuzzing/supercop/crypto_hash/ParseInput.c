#include "serialize.h"
#include "API.h"
#include "../../utilities/bufutils.h"
#include "../../utilities/fmt_str_parser.h"
#include "Call.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    // make sure an output file is specified
    if (argc < 2)
    {
        printf("Usage: %s in_fn.bin\n", argv[0]);
        return 0;
    }

    u8 *buf;
    load(argv[1], &buf);
    pp_t pp;
    fmt_t fmt;
    in_t x;
    in_t xp;
    out_t y;
    out_t yp;
    exp_res_t expres;
    unserialize(buf, &pp, &fmt, &x, &xp, &y, &expres);

    Call(&yp, &xp, &pp, &fmt);
    printf("x:\t");
    print_buffer_hex(x.buf, x.bytes);
    printf("xp:\t");
    print_buffer_hex(xp.buf, xp.bytes);
    printf("input bytelen:\t%lu\n", bits_to_bytes(fmt.list[1].bitlen));
    printf("y:\t");
    print_buffer_hex(y.buf, y.bytes);
    printf("yp:\t");
    print_buffer_hex(yp.buf, yp.bytes);
    printf("expected: %s\n", expres ? "equal" : "unequal");
}