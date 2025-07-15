#pragma once

#include "types.h"

typedef uint64_t len_t; // for ease of parsing in python

typedef struct {
    len_t bytes;
    u8 *buf;
} buf_t;

typedef struct {
    len_t list_len;
    buf_t *list;
} buf_list_t;

void buf_list_init(buf_list_t *buf_li, len_t list_len);
void buf_list_el(buf_t *buf_el, buf_list_t *list, len_t idx);
void buf_list_free(buf_list_t *buf_li);