#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <oqs/oqs.h>


int main(int argc, char **argv)
{
    printf("OQS_KEM_algs_length: %d\n", OQS_KEM_algs_length);
    printf("OQS_SIG_algs_length: %d\n", OQS_SIG_algs_length);
    printf("\n");

    size_t alg_id = 0;
    const char *name;
    while ((name = OQS_KEM_alg_identifier(alg_id)) != NULL)
    {
        printf("KEM %lu: %s\n", alg_id, name);
        alg_id++;
    }
    printf("\n");

    alg_id = 0;
    while ((name = OQS_SIG_alg_identifier(alg_id)) != NULL)
    {
        printf("SIG %lu: %s\n", alg_id, name);
        alg_id++;
    }
    return 0;
}