#include "../../../../../utilities/types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <oqs/oqs.h>

#ifndef ALG_ID
#define ALG_ID 0
#endif

__AFL_FUZZ_INIT();
int main(int argc, char **argv)
{
    u8 *shared_secret_e = NULL;
    u8 *ciphertext = NULL;
    u8 *pk = NULL;
    OQS_KEM *kem = NULL;
    const char *kem_name = OQS_KEM_alg_identifier(ALG_ID);
#ifdef __AFL_HAVE_MANUAL_CONTROL
  __AFL_INIT();
#endif
    unsigned char *buf = __AFL_FUZZ_TESTCASE_BUF;
    while (__AFL_LOOP(10000)) {
        int len = __AFL_FUZZ_TESTCASE_LEN;
        kem = OQS_KEM_new(kem_name);
        if (len != kem->length_public_key) {
            continue;
        }
        // size = size < kem->length_public_key? size : kem->length_public_key;
        shared_secret_e = calloc(kem->length_shared_secret, 1);
        ciphertext = calloc(kem->length_ciphertext, 1);
        pk = calloc(kem->length_public_key, 1);
        memcpy(pk, buf, kem->length_public_key);
        
        OQS_STATUS rv_encaps = OQS_KEM_encaps(kem, ciphertext, shared_secret_e, pk);
        free(pk);
        free(ciphertext);
        free(shared_secret_e);
        OQS_KEM_free(kem);
    }
    return 0;
}
