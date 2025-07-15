#ifndef __APPROX_MEMCMP_H_
#define __APPROX_MEMCMP_H_
#include <stddef.h>
int approx_memcmp(const void *s1, const void *s2, size_t n, double thresh_prob);
#endif
