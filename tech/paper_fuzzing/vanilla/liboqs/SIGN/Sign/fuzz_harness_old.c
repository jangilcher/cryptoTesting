#include "../../../../utilities/types.h"
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
    {
        return 1;
    }
    OQS_SIG *sig = NULL;
    const char *sig_name = OQS_SIG_alg_identifier(ALG_ID);
    sig = OQS_SIG_new(sig_name);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    size_t m_len, sk_len;
    if (size < sig->length_secret_key) {
        return 0;
    }
    m_len = size - sig->length_secret_key;
    sk_len = sig->length_secret_key;
    #ifdef OQS_VERSION_NUMBER
    #if OQS_VERSION_NUMBER == 0x0201811fL
    size_t length_signature = sig->length_sig_overhead;
    #endif
    #else
    size_t length_signature = sig->length_signature;
    #endif
    u8 *message = calloc(m_len, 1);
    u8 *sk = calloc(sig->length_secret_key, 1);
    u8 *signature = calloc(length_signature, 1);
    fread(sk, 1, sig->length_secret_key, fp);
    fread(message, 1, m_len, fp);
    fclose(fp);

    OQS_STATUS rv_sign = OQS_SIG_sign(sig, signature, &length_signature, message, m_len, sk);
    free(signature);
    free(sk);
    free(message);
    OQS_SIG_free(sig);
}
