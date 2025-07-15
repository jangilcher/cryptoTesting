#include "../../../../utilities/types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <oqs/oqs.h>

#ifndef ALG_ID
#define ALG_ID 0
#endif

__AFL_FUZZ_INIT();
int main(int argc, char **argv)
{   
    u8 *message = NULL;
    u8 *sk = NULL;
    u8 *signature = NULL;
    OQS_SIG *sig = NULL;
    const char *sig_name = OQS_SIG_alg_identifier(ALG_ID);
#ifdef __AFL_HAVE_MANUAL_CONTROL
  __AFL_INIT();
#endif
    unsigned char *buf = __AFL_FUZZ_TESTCASE_BUF;
    while (__AFL_LOOP(10000)) {
        int len = __AFL_FUZZ_TESTCASE_LEN;
        sig = OQS_SIG_new(sig_name);
        size_t m_len, sk_len;
        if (len < sig->length_secret_key) {
            return 0;
        }
        m_len = len - sig->length_secret_key;
        sk_len = sig->length_secret_key;
        #ifdef OQS_VERSION_NUMBER
        #if OQS_VERSION_NUMBER == 0x0201811fL
        size_t length_signature = sig->length_sig_overhead;
        #endif
        #else
        size_t length_signature = sig->length_signature;
        #endif
        message = calloc(m_len, 1);
        sk = calloc(sig->length_secret_key, 1);
        signature = calloc(length_signature, 1);
        memcpy(sk, buf, sig->length_secret_key);
        memcpy(message, buf+sk_len, m_len);

        OQS_STATUS rv_sign = OQS_SIG_sign(sig, signature, &length_signature, message, m_len, sk);
        free(signature);
        free(sk);
        free(message);
        OQS_SIG_free(sig);
    }
    return 0;
}
