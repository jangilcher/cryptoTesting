#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <sys/random.h>

// #define THRESH_PROB 0.001

/* Hamming weight computation based on 
 * http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable
 * by Sean Eron Anderson
*/

static const unsigned char BitsSetTable256[256] = 
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

int approx_memcmp(const void *s1, const void *s2, size_t n, double thresh_prob) {
    int c = 0; // c is the total bits set in v
    unsigned char * p1 = (unsigned char *) s1;
    unsigned char * p2 = (unsigned char *) s2;
    int bits = n*8;

    for (size_t i = 0; i < n; i++) {
        c += BitsSetTable256[p1[i] ^ p2[i]];
    }

    double t = (sqrt(log(thresh_prob) / (-2.0 * bits)) - 0.5) * (-bits);
    int threshold = floor(t);
    printf("bits=%d\nt= %f\nc= %d\nlower=%d\nupper=%d\n", bits, t, c, threshold, bits-threshold);
    return c < threshold || c > (bits-threshold);
}

// int main() {
//     char buf[32];
//     char buf2[32] = {0};
//     getrandom(buf, sizeof(buf), 0);
//     getrandom(buf2, sizeof(buf2), 0);
//     int ret = approx_memcmp(buf, buf2, sizeof(buf));
//     printf("ret=%d\n", ret);
// }