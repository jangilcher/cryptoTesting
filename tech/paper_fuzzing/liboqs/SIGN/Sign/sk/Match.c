#include "Match.h"
#include "../../../Call.h"
#include "../../../serialize.h"
#include "../../../../utilities/bufutils.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#define LOG(_fp, _fn, ...) do { (_fp) = fopen((_fn), "a+"); fprintf ((_fp), __VA_ARGS__); fclose((_fp)); } while (0)

void Match(out_t *y, out_t *yp, exp_res_t exp_res, aux_t aux)
{
    // if exp_res == true, y and yp should be equal
    // if exp_res == false, y and yp should be different

    size_t cmp_len = 0;
    memcpy(&cmp_len, aux.list[2].buf, sizeof(size_t));

    bool y_equals_yp = true;
    // NOTE: may or may not encode these as a equality check function
    //       in a hypothetical API.c file
    y_equals_yp &= (bool)(y->bytes == yp->bytes);
    y_equals_yp &= (bool)(y->retval == yp->retval);
    y_equals_yp &= (bool)(memcmp(y->buf, yp->buf, cmp_len) == 0);

    FILE *fp;
    // LOG(fp, "log", "DEBUG: hello from match\n");

    // printf("y : "); print_buffer_hex(y->buf, y->bytes);
    // printf("y': "); print_buffer_hex(yp->buf, yp->bytes);
    // printf("[y == yp]: %u\n", y_equals_yp);
    // printf("exp_res: %u\n", exp_res);

    if (y_equals_yp != exp_res)
    {
        abort();
    }
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
#include <assert.h>

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size, uint8_t *_xp, uint64_t _xp_len)
{
    pp_t pp;
    aux_t aux;
    fmt_t fmt;
    in_t x;
    in_t xp;
    out_t y;
    out_t yp;
    exp_res_t exp_res;

    FILE *fp = fopen("LLVMFuzzerTestOneInput.log", "a+");
    // print_buffer_hex_to_fp(fp, (u8 *)Data, Size);
    fclose(fp);

    unserialize((u8 *)Data, &pp, &aux, &fmt, &x, &xp, &y, &exp_res);

    if (_xp_len > 0)
    {
        assert(memcmp(x.buf, xp.buf, x.bytes) == 0);
        for (uint64_t i = 0; i < _xp_len; i++) {
            if (i >= xp.bytes) {
                break;
            }
            xp.buf[i] ^= _xp[i]; // if we store a mask x xor x'
            // xp.buf[i] = _xp[i]; // if we store directly x'
        }
        exp_res = (bool)(memcmp(x.buf+1, xp.buf+1, x.bytes-2) == 0);
    }

    Call(&yp, &xp, &pp, &aux, &fmt);

    // compare y and yp, crashes if check fails
    Match(&y, &yp, exp_res, aux);

    buf_list_free(&aux);
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
    bool using_afl = false;
    switch (argc)
    {
        case 2:
            // printf("Assuming `make run_c` or `make run_python` is being used.\n");
            break;
        case 3:
            // printf("Assuming `make run_afl` is being used.\n");
            using_afl = true;
            break;
        default:
            printf("Usage: %s in_fn.bin [mask_fn.bin]\n", argv[0]);
            return 0;
    }

    u8 *_buf = NULL;
    u64 _read = 0;
    if (using_afl)
    {
        char *_fn = argv[2];
        _read = load(_fn, &_buf);
    }

    char *fn = argv[1];
    u8 *buf;
    u64 read = load(fn, &buf);

    int retval = LLVMFuzzerTestOneInput(buf, read, _buf, _read);

    // LOG(fp, "log.match", "LLVMFuzzerTestOneInput returned %d.\n", retval);
    #if defined(STAND_ALONE)
    printf("LLVMFuzzerTestOneInput returned %d.\n", retval);
    #endif

    free(buf);
    if (_buf != NULL)
    {
        free(_buf);
    }
    return 0;
}
#endif
