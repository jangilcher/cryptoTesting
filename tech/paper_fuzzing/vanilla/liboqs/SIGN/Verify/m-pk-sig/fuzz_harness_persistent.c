#include "../../../../../utilities/types.h"
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
    #ifdef OQS_VERSION_NUMBER
    #if OQS_VERSION_NUMBER == 0x0201811fL
    // NOT SUPPORTED, old liboqs uses sign_open rather than verify, and the internals change per scheme
    fprintf(stderr, "liboqs version not supported.\n");
    exit(1);
    #endif
    #endif
    u8 *message = NULL;
    u8 *pk = NULL;
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
        size_t m_len, pk_len, sig_len;
        #ifdef OQS_VERSION_NUMBER
        #if OQS_VERSION_NUMBER == 0x0201811fL
        size_t length_signature = sig->length_sig_overhead;
        #endif
        #else
        size_t length_signature = sig->length_signature;
        #endif
        if (len < length_signature+sig->length_public_key) {
            continue;
        }
        m_len = len - length_signature - sig->length_public_key;
        pk_len = sig->length_public_key;
        sig_len = length_signature;
        message = calloc(m_len, 1);
        pk = calloc(pk_len, 1);
        signature = calloc(sig_len, 1);
        memcpy(pk, buf, pk_len);
        memcpy(signature, buf+pk_len, sig_len);
        memcpy(message, buf+pk_len+sig_len, m_len);
        OQS_STATUS rv_sign = OQS_SIG_verify(sig, message, m_len, signature, sig_len, pk);
        
        free(signature);
        free(pk);
        free(message);
        OQS_SIG_free(sig);
    }
    return 0;
}
