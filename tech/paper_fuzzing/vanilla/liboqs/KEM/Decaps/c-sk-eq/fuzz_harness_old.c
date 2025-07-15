#include "../../../../../utilities/types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


#include <oqs/oqs.h>

#ifndef ALG_ID
#define ALG_ID 0
#endif

int main(int argc, char **argv)
{
    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
        return 1;
    OQS_KEM *kem = NULL;
    const char *kem_name = OQS_KEM_alg_identifier(ALG_ID);
    kem = OQS_KEM_new(kem_name);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    if (size != kem->length_secret_key + kem->length_ciphertext)
        return 0;
    u8 *shared_secret_d = calloc(kem->length_shared_secret, 1);
    u8 *ciphertext = calloc(kem->length_ciphertext, 1);
    u8 *sk = calloc(kem->length_secret_key, 1);
    fread(sk, 1, kem->length_secret_key, fp);
    fread(ciphertext, 1, kem->length_ciphertext, fp);
    fclose(fp);
    OQS_STATUS rv_encaps = OQS_KEM_decaps(kem, shared_secret_d, ciphertext, sk);

    free(sk);
    free(ciphertext);
    free(shared_secret_d);
    OQS_KEM_free(kem);
}
