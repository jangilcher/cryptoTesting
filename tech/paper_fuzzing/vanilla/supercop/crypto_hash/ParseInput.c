#include "crypto_hash.h"
#include "../../../utilities/bufutils.h"
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
    size_t lenbytes;
    size_t buffbytes;
    unsigned long long inbuff_len = 0;
    if (size > sizeof(unsigned long long)) {
        lenbytes = sizeof(unsigned long long);
        buffbytes = size - lenbytes;
    } else {
        lenbytes = size;
        buffbytes = 0;
    }
    unsigned char * inbuff = calloc(buffbytes, 1);
    unsigned char * outbuff = calloc(crypto_hash_BYTES, 1);
    fread(inbuff, 1, buffbytes, fp);
    fread(&inbuff_len, 1, lenbytes, fp);
    fclose(fp);
    if (inbuff_len > buffbytes){
        exit(1);
    }

    printf("len:\t");
    print_buffer_hex(&inbuff_len, lenbytes);
    printf("m:\t");
    print_buffer_hex(inbuff, buffbytes);

    free(inbuff);
    free(outbuff);
}
