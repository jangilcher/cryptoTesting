#pragma once

#include <inttypes.h>
#include <stdbool.h>

typedef uint64_t len_t;
typedef int64_t rv_t;

typedef struct {
    len_t out_bitlen;
} pp_t;

typedef struct {
    uint8_t *buf;
    len_t bytes; // applications use bytes, so fair to store _outputs_ in bytes
    rv_t retval; // assuming int is general enough..
} out_t;

typedef struct {
    uint8_t *buf;
    len_t bytes; // inputs may be encoded in bits though!
} in_t;

typedef bool exp_res_t;