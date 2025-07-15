#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3)
    {
        printf("Usage: %s out_fn.bin xbytelen\n", argv[0]);
        return 0;
    }
    FILE *fp = fopen(argv[1], "wb");
    if (!fp)
        return 1;
    
    size_t bytelen = atoll(argv[2]);
    unsigned char * message = calloc(bytelen, 1);
    unsigned long long len = sizeof(message);
    fwrite(message, 1, sizeof(message), fp);
    // fwrite(&len, 1, sizeof(unsigned long long), fp);
    fclose(fp);
}
