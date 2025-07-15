#include "buf_list.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

void buf_list_init(buf_list_t *buf_li, len_t list_len)
{
    buf_li->list_len = list_len;
    buf_li->list = (buf_t *) calloc(list_len, sizeof(buf_t)); // important to set to 0 so that .buf == NULL, see buf_list_free
}

void buf_list_el(buf_t *buf_el, buf_list_t *list, len_t idx)
{
    assert(idx >= 0);
    assert(idx < list->list_len);
    buf_el->bytes = list->list[idx].bytes;
    buf_el->buf = (u8 *)malloc(buf_el->bytes);
    memcpy(buf_el->buf, list->list[idx].buf, buf_el->bytes);
}

void buf_list_free(buf_list_t *buf_li)
{
    if (buf_li == NULL)
    {
        return;
    }

    for (len_t i = 0; i < buf_li->list_len; i++)
    {
        if (buf_li->list[i].buf != NULL)
        {
            free(buf_li->list[i].buf);
        }
    }
    free(buf_li->list);
}

#if defined(UNIT_TESTS)

#include "../tests/minunit.h"
int tests_run = 0;

buf_list_t init_buf_list(int list_len)
{
    // populate buf_li = [ [0], [1, 1], [2, 2, 2], ...]
    buf_list_t buf_li;
    buf_list_init(&buf_li, list_len);
    // buf_li.list_len = list_len;
    // buf_li.list = (buf_t *)malloc(buf_li.list_len * sizeof(buf_t));

    for (int i = 0; i < buf_li.list_len; i++)
    {
        buf_li.list[i].bytes = i+1;
        buf_li.list[i].buf = (u8 *)malloc(buf_li.list[i].bytes);
        for (int j = 0; j < buf_li.list[i].bytes; j++)
        {
            buf_li.list[i].buf[j] = i;
        }
    }

    return buf_li;
}

static char * test_buf_list_el()
{
    buf_list_t buf_li = init_buf_list(4);
    for (int i = 0; i < buf_li.list_len; i++)
    {
        buf_t el;
        buf_list_el(&el, &buf_li, i);
        bool expected_res = true;
        for (int j = 0; j < el.bytes; j++)
        {
            expected_res = (expected_res && (bool)(el.buf[j] == i));
            mu_assert("error, list[i] != [i, i, i, ..., i]", expected_res == true);
        }
        free(el.buf);
    }

    buf_list_free(&buf_li);
    // for (int i = 0; i < buf_li.list_len; i++)
    // {
    //     free(buf_li.list[i].buf);
    // }
    // free(buf_li.list);
    return 0;
}

static char * all_tests()
{
    mu_run_test(test_buf_list_el);
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
