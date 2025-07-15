#pragma once

#include "types.h"

typedef enum { EQ = 1, DIFF = 0 } lbl_t;

typedef uint64_t len_t; // for ease of parsing in python

typedef struct {
    len_t bitlen;
    lbl_t lbl;
} tuple_t;

typedef struct {
    tuple_t *list;
    len_t list_len;
} fmt_t;

uint64_t BufBitlen(fmt_t *fmt);
uint64_t BufBytelen(fmt_t *fmt);
lbl_t GetLabel(uint64_t i, fmt_t *fmt);
