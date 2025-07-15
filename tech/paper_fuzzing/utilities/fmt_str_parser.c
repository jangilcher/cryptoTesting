#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "fmt_str_parser.h"

u64 BufBitlen(fmt_t *fmt)
{
    u64 bitlen = 0;
    for (u64 i = 0; i < fmt->list_len; i++)
    {
        bitlen += fmt->list[i].bitlen;
    }
    return bitlen;
}

u64 BufBytelen(fmt_t *fmt)
{
    u64 bits = BufBitlen(fmt);
    u64 bytes = bits_to_bytes(bits);
    return bytes;
}


lbl_t GetLabel(u64 i, fmt_t *fmt)
{
    assert(fmt->list_len != 0);
    // NOTE: in order to keep u64 everywere, the wile loop differs from the paper
    u64 cnt = 0; // will roll over
    u64 idx = -1; // will roll over
    lbl_t lbl;
    while (cnt <= i)
    {
        idx++;
        assert(idx < fmt->list_len);
        u64 ell = fmt->list[idx].bitlen;
        lbl = fmt->list[idx].lbl;
        cnt += ell;
    };
    return lbl;
}

#if defined(UNIT_TESTS)

#include "../tests/minunit.h"
int tests_run = 0;

fmt_t init_fmt()
{
    fmt_t fmt;
    fmt.list_len = 4;
    fmt.list = (tuple_t *)malloc(fmt.list_len * sizeof(tuple_t));

    fmt.list[0].bitlen = 0;
    fmt.list[0].lbl = EQ;

    fmt.list[1].bitlen = 6;
    fmt.list[1].lbl = DIFF;

    // final bits of last bite of x
    fmt.list[2].bitlen = 8 * bits_to_bytes(fmt.list[1].bitlen) - fmt.list[1].bitlen;
    fmt.list[2].lbl = EQ;

    fmt.list[3].bitlen = 8;
    fmt.list[3].lbl = EQ;
    
    return fmt;
}

fmt_t init_fmt2()
{
    fmt_t fmt;
    fmt.list_len = 4;
    fmt.list = (tuple_t *)malloc(fmt.list_len * sizeof(tuple_t));

    fmt.list[0].bitlen = 8;
    fmt.list[0].lbl = EQ;

    fmt.list[1].bitlen = 6;
    fmt.list[1].lbl = DIFF;

    // final bits of last bite of x
    fmt.list[2].bitlen = 8 * bits_to_bytes(fmt.list[1].bitlen) - fmt.list[1].bitlen;
    fmt.list[2].lbl = EQ;

    fmt.list[3].bitlen = 8;
    fmt.list[3].lbl = EQ;
    
    return fmt;
}

fmt_t empty_fmt()
{
    fmt_t fmt;
    fmt.list_len = 0;
    fmt.list = NULL;    
    return fmt;
}

static char * test_BufBitlen()
{
    fmt_t fmt = init_fmt();
    mu_assert("error, BufBitlen(fmt) != 24", BufBitlen(&fmt) == 16);
    free(fmt.list);

    fmt = init_fmt2();
    mu_assert("error, BufBitlen(fmt) != 24", BufBitlen(&fmt) == 24);
    free(fmt.list);

    fmt_t empty = empty_fmt();
    mu_assert("error, BufBitlen(empty) != 0", BufBitlen(&empty) == 0);
    return 0;
}

static char * test_BufBytelen()
{
    fmt_t fmt = init_fmt();
    mu_assert("error, BufBytelen(fmt) != 3", BufBytelen(&fmt) == 2);
    free(fmt.list);

    fmt = init_fmt2();
    mu_assert("error, BufBytelen(fmt) != 3", BufBytelen(&fmt) == 3);
    free(fmt.list);

    fmt_t empty = empty_fmt();
    mu_assert("error, BufBytelen(empty) != 0", BufBytelen(&empty) == 0);
    return 0;
}

static char * test_GetLabel()
{
    fmt_t fmt = init_fmt();
    mu_assert("error, GetLabel(0, fmt) != EQ",   GetLabel(0, &fmt) != EQ);
    mu_assert("error, GetLabel(0, fmt) == DIFF", GetLabel(0, &fmt) == DIFF);
    mu_assert("error, GetLabel(1, fmt) != EQ",   GetLabel(1, &fmt) != EQ);
    mu_assert("error, GetLabel(1, fmt) == DIFF", GetLabel(1, &fmt) == DIFF);
    mu_assert("error, GetLabel(5, fmt) != EQ",   GetLabel(5, &fmt) != EQ);
    mu_assert("error, GetLabel(5, fmt) == DIFF", GetLabel(5, &fmt) == DIFF);
    mu_assert("error, GetLabel(6, fmt) == EQ",   GetLabel(6, &fmt) == EQ);
    mu_assert("error, GetLabel(6, fmt) != DIFF", GetLabel(6, &fmt) != DIFF);
    mu_assert("error, GetLabel(7, fmt) == EQ",   GetLabel(7, &fmt) == EQ);
    mu_assert("error, GetLabel(7, fmt) != DIFF", GetLabel(7, &fmt) != DIFF);
    mu_assert("error, GetLabel(8, fmt) == EQ",   GetLabel(8, &fmt) == EQ);
    mu_assert("error, GetLabel(8, fmt) != DIFF", GetLabel(8, &fmt) != DIFF);
    mu_assert("error, GetLabel(15, fmt) == EQ",   GetLabel(15, &fmt) == EQ);
    mu_assert("error, GetLabel(15, fmt) != DIFF", GetLabel(15, &fmt) != DIFF);
    // if (fmt.list[1].bitlen == 6)
    // {
    //     mu_assert("error, GetLabel(14, fmt) != EQ",   GetLabel(14, &fmt) == EQ);
    //     mu_assert("error, GetLabel(14, fmt) == DIFF", GetLabel(14, &fmt) != DIFF);
    //     mu_assert("error, GetLabel(15, fmt) != EQ",   GetLabel(15, &fmt) == EQ);
    //     mu_assert("error, GetLabel(15, fmt) == DIFF", GetLabel(15, &fmt) != DIFF);
    // }
    // mu_assert("error, GetLabel(BufBitlen(&fmt)-1, fmt) != EQ",   GetLabel(BufBitlen(&fmt)-1, &fmt) == EQ);
    // mu_assert("error, GetLabel(BufBitlen(&fmt)-1, fmt) == DIFF", GetLabel(BufBitlen(&fmt)-1, &fmt) != DIFF);
    // mu_assert("error, GetLabel(BufBitlen(&fmt), fmt) != EQ",   GetLabel(BufBitlen(&fmt), &fmt) == EQ); // this fails with Assertion error as expected
    // mu_assert("error, GetLabel(BufBitlen(&fmt), fmt) == DIFF", GetLabel(BufBitlen(&fmt), &fmt) != DIFF); // this fails with Assertion error as expected
    // mu_assert("error, GetLabel(150, fmt) != EQ",   GetLabel(150, &fmt) == EQ);  // this fails with Assertion error as expected
    // mu_assert("error, GetLabel(150, fmt) == DIFF", GetLabel(150, &fmt) != DIFF);  // this fails with Assertion error as expected
    free(fmt.list);

    fmt = init_fmt2();
    mu_assert("error, GetLabel(0, fmt) != EQ",   GetLabel(0, &fmt) == EQ);
    mu_assert("error, GetLabel(0, fmt) == DIFF", GetLabel(0, &fmt) != DIFF);
    mu_assert("error, GetLabel(1, fmt) != EQ",   GetLabel(1, &fmt) == EQ);
    mu_assert("error, GetLabel(1, fmt) == DIFF", GetLabel(1, &fmt) != DIFF);
    mu_assert("error, GetLabel(7, fmt) != EQ",   GetLabel(7, &fmt) == EQ);
    mu_assert("error, GetLabel(7, fmt) == DIFF", GetLabel(7, &fmt) != DIFF);
    mu_assert("error, GetLabel(8, fmt) == EQ",   GetLabel(8, &fmt) != EQ);
    mu_assert("error, GetLabel(8, fmt) != DIFF", GetLabel(8, &fmt) == DIFF);
    mu_assert("error, GetLabel(13, fmt) == EQ",   GetLabel(13, &fmt) != EQ);
    mu_assert("error, GetLabel(13, fmt) != DIFF", GetLabel(13, &fmt) == DIFF);
    if (fmt.list[1].bitlen == 6)
    {
        mu_assert("error, GetLabel(14, fmt) != EQ",   GetLabel(14, &fmt) == EQ);
        mu_assert("error, GetLabel(14, fmt) == DIFF", GetLabel(14, &fmt) != DIFF);
        mu_assert("error, GetLabel(15, fmt) != EQ",   GetLabel(15, &fmt) == EQ);
        mu_assert("error, GetLabel(15, fmt) == DIFF", GetLabel(15, &fmt) != DIFF);
    }
    mu_assert("error, GetLabel(BufBitlen(&fmt)-1, fmt) != EQ",   GetLabel(BufBitlen(&fmt)-1, &fmt) == EQ);
    mu_assert("error, GetLabel(BufBitlen(&fmt)-1, fmt) == DIFF", GetLabel(BufBitlen(&fmt)-1, &fmt) != DIFF);
    free(fmt.list);

    fmt_t empty = empty_fmt();
    // mu_assert("error, GetLabel(0, empty) != EQ", GetLabel(0, &empty) == EQ); // this fails with Assertion error as expected
    return 0;
}

static char * all_tests()
{
    mu_run_test(test_BufBitlen);
    mu_run_test(test_BufBytelen);
    mu_run_test(test_GetLabel);
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
