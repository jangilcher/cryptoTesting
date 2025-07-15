#include "../../../../../utilities/types.h"
#include "../../../../../utilities/liboqs_prng.h"
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
    FILE *fp = fopen(argv[1], "wb");
    if (!fp)
        return 1;
    char prg_seed[48];
    memset(prg_seed, 0, 48);
    sprintf(prg_seed, "geninput");

#ifdef CHES_VERSION
    OQS_randombytes_custom_algorithm(&OQS_randombytes_nist_kat);
#else
    OQS_randombytes_switch_algorithm(OQS_RAND_alg_nist_kat);
#endif
    OQS_randombytes_nist_kat_init_256bit((uint8_t *)prg_seed, NULL);

    OQS_KEM *kem = NULL;
    const char *kem_name = OQS_KEM_alg_identifier(ALG_ID);
    kem = OQS_KEM_new(kem_name);
    u8 *shared_secret_e = calloc(kem->length_shared_secret, 1);
    u8 *ciphertext = calloc(kem->length_ciphertext, 1);
    u8 *pk = calloc(kem->length_public_key, 1);
    u8 *sk = calloc(kem->length_secret_key, 1);

    OQS_STATUS rc = OQS_KEM_keypair(kem, pk, sk);
    if (rc != OQS_SUCCESS)
    {
        free(sk);
        free(pk);
        free(ciphertext);
        free(shared_secret_e);
        OQS_KEM_free(kem);
        return OQS_ERROR;
    }
    fwrite(pk, 1, kem->length_public_key, fp);
    fclose(fp);
}
