#define _GNU_SOURCE //reallocarray
#include "serialize.h"
#include "../../utilities/bufutils.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// All arguments must already be allocated before calling serialize
// NOTE: this assumes |x| = |xp|
u64 serialize(u8 **buf, pp_t *pp, fmt_t *fmt, in_t *x, in_t *xp, out_t *y, exp_res_t *expres)
{
    // compute total bytelen
    u64 bytes = 0;
    bytes += sizeof(pp_t);
    bytes += sizeof(fmt->list_len);
    bytes += sizeof(tuple_t) * fmt->list_len;
    bytes += sizeof(x->bytes);
    bytes += x->bytes * 2;
    bytes += sizeof(y->bytes);
    bytes += y->bytes;
    bytes += sizeof(y->retval);
    bytes += sizeof(exp_res_t);

    u64 offset = 0;

    // allocate buffer
    *buf = (u8 *) calloc(bytes, sizeof(u8));

    // serialise pp
    memcpy(*buf+offset, pp, sizeof(pp_t));
    offset += sizeof(pp_t);

    // serialise fmt
    memcpy(*buf+offset, &(fmt->list_len), sizeof(fmt->list_len));
    offset += sizeof(fmt->list_len);
    memcpy(*buf+offset, fmt->list, sizeof(tuple_t) * fmt->list_len);
    offset += sizeof(tuple_t) * fmt->list_len;

    // serialise x twice
    memcpy(*buf+offset, &(x->bytes), sizeof(x->bytes));
    offset += sizeof(x->bytes);
    memcpy(*buf+offset, x->buf, x->bytes);
    offset += x->bytes;
    memcpy(*buf+offset, xp->buf, x->bytes); // NOTE: this assumes |x| = |xp|
    offset += x->bytes;

    // serialise y
    memcpy(*buf+offset, &(y->bytes), sizeof(y->bytes));
    offset += sizeof(y->bytes);
    memcpy(*buf+offset, y->buf, y->bytes);
    offset += y->bytes;
    memcpy(*buf+offset, &(y->retval), sizeof(y->retval));
    offset += sizeof(y->retval);

    // serialize expres
    memcpy(*buf+offset, expres, sizeof(exp_res_t));
    offset += sizeof(exp_res_t);

    assert(bytes == offset);

    return bytes;
}

// All arguments must already be allocated before calling serialize
// However, buffers pointed to by the structs (say, `buf` inside `x`) should not be.
// NOTE: if this is changed, update serialize.py accordingly
void unserialize(u8 *buf, pp_t *pp, fmt_t *fmt, in_t *x, in_t *xp, out_t *y, exp_res_t *expres)
{
    u64 offset = 0;

    // unserialise pp
    memcpy(pp, buf+offset, sizeof(pp_t));
    offset += sizeof(pp_t);

    // unserialise fmt
    memcpy(&(fmt->list_len), buf+offset, sizeof(fmt->list_len));
    offset += sizeof(fmt->list_len);
    fmt->list = (tuple_t *) calloc(fmt->list_len, sizeof(tuple_t));
    memcpy(fmt->list, buf+offset, sizeof(tuple_t) * fmt->list_len);
    offset += sizeof(tuple_t) * fmt->list_len;

    // unserialise x
    memcpy(&(x->bytes), buf+offset, sizeof(x->bytes));
    offset += sizeof(x->bytes);
    x->buf = (u8 *) calloc(x->bytes, sizeof(u8));
    memcpy(x->buf, buf+offset, x->bytes);
    offset += x->bytes;

    // unserialise x'
    xp->bytes = x->bytes;
    xp->buf = (u8 *) calloc(xp->bytes, sizeof(u8));
    memcpy(xp->buf, buf+offset, xp->bytes);
    offset += xp->bytes;

    // unserialise y
    memcpy(&(y->bytes), buf+offset, sizeof(y->bytes));
    offset += sizeof(y->bytes);
    y->buf = (u8 *) calloc(y->bytes, sizeof(u8));
    memcpy(y->buf, buf+offset, y->bytes);
    offset += y->bytes;
    memcpy(&(y->retval), buf+offset, sizeof(y->retval));
    offset += sizeof(y->retval);

    // unserialize expres
    memcpy(expres, buf+offset, sizeof(exp_res_t));
    // offset += sizeof(exp_res_t);
}

void dump(const char *fn, u8 *buf, u64 bytes)
{
    FILE *fp;

    fp = fopen(fn, "wb");
    if (!fp)
    {
        printf("ERROR: Unable to open file.");
        exit(1);
    }
    fwrite(buf, bytes, 1, fp);
    fclose(fp);
}

// NOTE: the address stored in *buf will be reset!
u64 load(const char *fn, u8 **buf)
{
    FILE *fp;

    fp = fopen(fn, "rb");
    if (!fp)
    {
        printf("ERROR: Unable to open file.");
        exit(1);
    }

    u64 chunk_len = 4;
    u64 read_now = 0;
    u64 read_tot = 0;
    *buf = NULL;
    do {
        *buf = (u8 *) reallocarray(*buf, read_tot + chunk_len, 1);
        if (*buf == NULL)
        {
            printf("ERROR: out of memory while reading from disk.\n");
            exit(1);
        }
        read_now = (u64)fread(*buf + read_tot, 1, chunk_len, fp);
        read_tot += read_now;
    } while (read_now == chunk_len);

    // free final extra bytes, if any
    *buf = (u8 *) reallocarray(*buf, read_tot, 1);

    fclose(fp);

    // print_buffer_hex(*buf, read_tot);

    return read_tot;
}

off_t fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

#if defined(UNIT_TESTS)

#include <unistd.h>
#include <string.h>
#include "../tests/minunit.h"
int tests_run = 0;

pp_t init_pp()
{
    pp_t pp;
    pp.out_bitlen = 224;
    return pp;
}

fmt_t init_fmt()
{
    fmt_t fmt;
    fmt.list_len = 4;
    fmt.list = (tuple_t *)malloc(fmt.list_len * sizeof(tuple_t));

    fmt.list[0].bitlen = 8;
    fmt.list[0].lbl = EQ;

    fmt.list[1].bitlen = 6;
    fmt.list[1].lbl = DIFF;

    fmt.list[2].bitlen = 8 * bits_to_bytes(fmt.list[1].bitlen) - fmt.list[1].bitlen;
    fmt.list[2].lbl = EQ;

    fmt.list[3].bitlen = 8;
    fmt.list[3].lbl = EQ;

    return fmt;
}

in_t init_x(fmt_t *fmt)
{
    in_t x;
    x.bytes = 0;
    for (u64 el = 0; el < fmt->list_len; el++)
    {
        x.bytes += bits_to_bytes(fmt->list[el].bitlen);
    }
    x.buf = (u8 *) malloc(x.bytes);
    for (u64 el = 0; el < x.bytes; el++)
    {
        x.buf[el] = (u8) el;
    }
    return x;
}

out_t init_y(pp_t *pp)
{
    out_t y;
    y.bytes = bits_to_bytes(pp->out_bitlen);
    y.buf = (u8 *)malloc(y.bytes);
    for (u64 el = 0; el < y.bytes; el++)
    {
        y.buf[el] = (u8) el;
    }
    y.retval = 0xdeadbeefdeadbeef;
    return y;
}

static char * test_serialize()
{
    char fn[] = "tmp.bin";

    // check a file does not exist first
    if( access( fn, F_OK ) == 0 ) {
        return "error, temporary test file already exists";
    }

    fmt_t fmt = init_fmt();
    pp_t pp = init_pp();
    in_t x = init_x(&fmt);
    out_t y = init_y(&pp);
    exp_res_t expres = DIFF;
    u8 *buf;
    u64 bytes;

    bytes = serialize(&buf, &pp, &fmt, &x, &x, &y, &expres);
    dump(fn, buf, bytes);
    printf("bytes dumped: %lu\n", bytes);

    fmt_t _fmt;
    pp_t _pp;
    in_t _x;
    in_t _xp;
    out_t _y;
    exp_res_t _expres;
    u8 *_buf;

    u64 read = 0;
    read = load(fn, &_buf);
    printf("bytes read: %lu\n", read);
    unserialize(_buf, &_pp, &_fmt, &_x, &_xp, &_y, &_expres);

    mu_assert("error, fmt.list_len != _fmt.list_len", memcmp(&fmt.list_len, &_fmt.list_len, sizeof(fmt.list_len)) == 0);
    mu_assert("error, fmt.list != _fmt.list", memcmp(fmt.list, _fmt.list, fmt.list_len * sizeof(tuple_t)) == 0);
    mu_assert("error, pp != _pp", memcmp(&pp, &_pp, sizeof(pp_t)) == 0);
    mu_assert("error, x.bytes != _x.bytes", memcmp(&x.bytes, &_x.bytes, sizeof(x.bytes)) == 0);
    mu_assert("error, x.buf != _x.buf", memcmp(x.buf, _x.buf, x.bytes) == 0);
    mu_assert("error, x.bytes != _xp.bytes", memcmp(&x.bytes, &_xp.bytes, sizeof(x.bytes)) == 0);
    mu_assert("error, x.buf != _xp.buf", memcmp(x.buf, _xp.buf, x.bytes) == 0);
    mu_assert("error, y.bytes != _y.bytes", memcmp(&y.bytes, &_y.bytes, sizeof(y.bytes)) == 0);
    mu_assert("error, y.buf != _y.buf", memcmp(y.buf, _y.buf, y.bytes) == 0);
    mu_assert("error, y.retval != _y.retval", memcmp(&y.retval, &_y.retval, sizeof(y.retval)) == 0);
    mu_assert("error, expres != _expres", memcmp(&expres, &_expres, sizeof(expres)) == 0);

    unlink(fn);

    free(x.buf);
    free(y.buf);
    free(fmt.list);
    free(buf);

    free(_x.buf);
    free(_xp.buf);
    free(_y.buf);
    free(_fmt.list);
    free(_buf);
    return 0;
}

static char * all_tests()
{
    mu_run_test(test_serialize);
    return 0;
}

int main(int argc, char **argv)
{
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}

#endif
