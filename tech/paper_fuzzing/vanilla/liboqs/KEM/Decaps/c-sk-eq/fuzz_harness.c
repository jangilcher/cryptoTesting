#include "../../../../../utilities/types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <oqs/oqs.h>

#ifndef ALG_ID
#define ALG_ID 0
#endif
__AFL_FUZZ_INIT();
int main(int argc, char **argv)
{
    u8 *shared_secret_d = NULL;
    u8 *ciphertext = NULL;
    u8 *sk = NULL;
    OQS_KEM *kem = NULL;
    const char *kem_name = OQS_KEM_alg_identifier(ALG_ID);
#ifdef __AFL_HAVE_MANUAL_CONTROL
  __AFL_INIT();
#endif
    unsigned char *buf = __AFL_FUZZ_TESTCASE_BUF;
    while (__AFL_LOOP(10000)) {
        int len = __AFL_FUZZ_TESTCASE_LEN;
        kem = OQS_KEM_new(kem_name);
        if (len != kem->length_secret_key + kem->length_ciphertext)
            continue;
        shared_secret_d = calloc(kem->length_shared_secret, 1);
        ciphertext = calloc(kem->length_ciphertext, 1);
        sk = calloc(kem->length_secret_key, 1);
        memcpy(sk, buf, kem->length_secret_key);
        memcpy(ciphertext, buf+kem->length_secret_key, kem->length_ciphertext);
        OQS_STATUS rv_encaps = OQS_KEM_decaps(kem, shared_secret_d, ciphertext, sk);

        free(sk);
        free(ciphertext);
        free(shared_secret_d);
        OQS_KEM_free(kem);
    }
    return 0;
}
