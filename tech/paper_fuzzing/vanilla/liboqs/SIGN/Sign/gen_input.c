#include "../../../../utilities/types.h"
#include "../../../../utilities/liboqs_prng.h"
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
    char prg_seed[48];
    memset(prg_seed, 0, 48);
    sprintf(prg_seed, "geninput");

#ifdef CHES_VERSION
    OQS_randombytes_custom_algorithm(&OQS_randombytes_nist_kat);
#else
    OQS_randombytes_switch_algorithm(OQS_RAND_alg_nist_kat);
#endif
    OQS_randombytes_nist_kat_init_256bit((uint8_t *)prg_seed, NULL);
    FILE *fp = fopen(argv[1], "wb");
    if (!fp)
        return 1;
    int ret_val = 0;
    OQS_SIG *sig = NULL;
    const char *sig_name = OQS_SIG_alg_identifier(ALG_ID);
    sig = OQS_SIG_new(sig_name);
    u8 *pk = calloc(sig->length_public_key, 1);
    u8 *sk = calloc(sig->length_secret_key, 1);
    u8 message[] ="Hello World!";

    OQS_STATUS rc = OQS_SIG_keypair(sig, pk, sk);
    if (rc != OQS_SUCCESS)
    {
        ret_val = OQS_ERROR;
        goto ERROR;
    }
    fwrite(sk, 1, sig->length_secret_key, fp);
    fwrite(message, 1, sizeof(message), fp);
    fclose(fp);

ERROR:
    free(sk);
    free(pk);
    OQS_SIG_free(sig);
    return ret_val;
}
