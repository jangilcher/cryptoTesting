#include "../../../../../utilities/types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <oqs/oqs.h>

#ifndef ALG_ID
#define ALG_ID 0
#endif

int main(int argc, char **argv)
{
    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
        return 1;
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    OQS_KEM *kem = NULL;
    const char *kem_name = OQS_KEM_alg_identifier(ALG_ID);
    kem = OQS_KEM_new(kem_name);
    if (size != kem->length_public_key) {
        return 0;
    }
    // size = size < kem->length_public_key? size : kem->length_public_key;
    u8 *shared_secret_e = calloc(kem->length_shared_secret, 1);
    u8 *ciphertext = calloc(kem->length_ciphertext, 1);
    u8 *pk = calloc(kem->length_public_key, 1);
    fread(pk, 1, kem->length_public_key, fp);
    fclose(fp);
    OQS_STATUS rv_encaps = OQS_KEM_encaps(kem, ciphertext, shared_secret_e, pk);
    free(pk);
    free(ciphertext);
    free(shared_secret_e);
    OQS_KEM_free(kem);
}
