#include "crypto_hash.h"
#include <stdlib.h>
#include <stdio.h>
typedef int ret_t;

#ifndef CRYPTO_BYTES
#define CRYPTO_BYTES 256
#endif

int main(int argc, char **argv)
{
    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
        return 1;
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    unsigned long long inbuff_len = 0;
    unsigned char * inbuff = calloc(size, 1);
    unsigned char * outbuff = calloc(crypto_hash_BYTES, 1);
    fread(inbuff, 1, size, fp);
    fclose(fp);
    ret_t retval = crypto_hash(outbuff, inbuff, size);
    return retval;
}
