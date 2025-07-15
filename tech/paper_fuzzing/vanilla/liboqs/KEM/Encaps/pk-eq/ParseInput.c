#include "../../../../../utilities/types.h"
#include "../../../../../utilities/bufutils.h"
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
    size_t pk_len;
    if (size != kem->length_public_key) {
        pk_len = size;
    } else {
        pk_len = kem->length_public_key;
    }
    // size = size < kem->length_public_key? size : kem->length_public_key;
    u8 *shared_secret_e = calloc(kem->length_shared_secret, 1);
    u8 *ciphertext = calloc(kem->length_ciphertext, 1);
    u8 *pk = calloc(pk_len, 1);
    fread(pk, 1, pk_len, fp);
    fclose(fp);

    printf("pk:\t");
    print_buffer_hex(pk, pk_len);
    printf("pk_len:\t%zu\n", pk_len);

    OQS_KEM_free(kem);
    free(pk);
    free(ciphertext);
    free(shared_secret_e);
}
