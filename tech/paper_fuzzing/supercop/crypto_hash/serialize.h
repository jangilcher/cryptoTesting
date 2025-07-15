#pragma once

#define member_size(type, member) sizeof(((type *)0)->member)

#include "API.h"
#include "../../utilities/fmt_str_parser.h"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>


u64 serialize(u8 **buf, pp_t *pp, fmt_t *fmt, in_t *x, in_t *xp, out_t *y, exp_res_t *expres);
void unserialize(u8 *buf, pp_t *pp, fmt_t *fmt, in_t *x, in_t *xp, out_t *y, exp_res_t *expres);
void dump(const char *fn, u8 *buf, u64 bytes);
u64 load(const char *fn, u8 **buf);
off_t fsize(const char *filename);
