#include "Match.h"
#include "Call.h"
#include "serialize.h"
#include "../../utilities/bufutils.h"
#include "../../utilities/approx_memcmp.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#define LOG(_fp, _fn, ...) do { (_fp) = fopen((_fn), "a+"); fprintf ((_fp), __VA_ARGS__); fclose((_fp)); } while (0)

void Match(out_t *y, out_t *yp, exp_res_t exp_res)
{
    // if exp_res == true, y and yp should be equal
    // if exp_res == false, y and yp should be different

    bool y_equals_yp = true;
    // NOTE: may or may not encode these as a equality check function
    //       in a hypothetical API.c file
    y_equals_yp &= (bool)(y->bytes == yp->bytes);
    y_equals_yp &= (bool)(y->retval == yp->retval);
    y_equals_yp &= (bool)(memcmp(y->buf, yp->buf, y->bytes) == 0);

    FILE *fp;
    LOG(fp, "log", "DEBUG: hello from match\n");

    // printf("y : "); print_buffer_hex(y->buf, y->bytes);
    // printf("y': "); print_buffer_hex(yp->buf, yp->bytes);
    // printf("[y == yp]: %u\n", y_equals_yp);
    // printf("exp_res: %u\n", exp_res);

    if (y_equals_yp != exp_res)
    {
        abort();
    }
}

int incmp(in_t* a, in_t* b, fmt_t * fmt) {
    //simplified! supercop api uses bytes so we only handle bytelengths
    len_t idx = 0;
    int res = 0;
    for (int i=0; i < fmt->list_len; i++) {
        len_t byte_len = fmt->list[i].bitlen / 8;
        if (byte_len > 0) {
            res |=  (fmt->list[i].lbl == DIFF) && memcmp(a->buf+idx, b->buf+idx, byte_len);
            idx += byte_len;
        }
    }
    return res;
}

/*
 * From https://www.llvm.org/docs/LibFuzzer.html regarding LLVMFuzzerTestOneInput:
 * - The fuzzing engine will execute the fuzz target many times with different inputs in the same process.
 * - It must tolerate any kind of input (empty, huge, malformed, etc).
 * - It must not exit() on any input.
 * - It may use threads but ideally all threads should be joined at the end of the function.
 * - It must be as deterministic as possible. Non-determinism (e.g. random decisions not based on the input bytes) will make fuzzing inefficient.
 * - It must be fast. Try avoiding cubic or greater complexity, logging, or excessive memory consumption.
 * - Ideally, it should not modify any global state (although that’s not strict).
 * - Usually, the narrower the target the better. E.g. if your target can parse several data formats, split it into several targets, one per format.
*/
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    fmt_t fmt;
    pp_t pp;
    in_t x;
    in_t xp;
    out_t y;
    out_t yp;
    exp_res_t exp_res;

    FILE *fp = fopen("log", "a+");
    print_buffer_hex_to_fp(fp, (u8 *)Data, Size);
    fclose(fp);

    unserialize((u8 *)Data, &pp, &fmt, &x, &xp, &y, &exp_res);

    Call(&yp, &xp, &pp, &fmt);

    // compare y and yp, crashes if check fails
    Match(&y, &yp, exp_res);

    // if (incmp(&x, &xp, &fmt)){
    //     // printf("Randomness test\n");
    //     if (approx_memcmp(y.buf, yp.buf, y.bytes, 0.01)) {
    //         fprintf(stdout, "not enough randomness in output\n");
    //         fflush(stdout);
    //         // fprintf(stderr, "not enough randomness\n");
    //         abort();
    //     }
    // }

    free(fmt.list);
    free(x.buf);
    free(xp.buf);
    free(y.buf);
    free(yp.buf);

    return 0;
}


#if !defined(USE_LIBFUZZER)
int main(int argc, char **argv)
{
    // FILE *fp; LOG(fp, "log.match", "argc: %d\n", argc);
    // for (int i = 0; i < argc; i++) LOG(fp, "log.match", "argv[%d]: %s\n", i, argv[i]);

    // make sure an output file is specified
    if (argc != 2)
    {
        printf("Usage: %s in_fn.bin\n", argv[0]);
        return 0;
    }

    char *fn = argv[1];
    u8 *buf;
    u64 read = load(fn, &buf);

    int retval = LLVMFuzzerTestOneInput(buf, read);

    // LOG(fp, "log.match", "LLVMFuzzerTestOneInput returned %d.\n", retval);
    #if defined(STAND_ALONE)
    printf("LLVMFuzzerTestOneInput returned %d.\n", retval);
    #endif

    free(buf);
    return 0;
}
#endif