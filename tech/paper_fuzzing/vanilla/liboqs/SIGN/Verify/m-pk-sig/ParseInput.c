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
    #ifdef OQS_VERSION_NUMBER
    #if OQS_VERSION_NUMBER == 0x0201811fL
    // NOT SUPPORTED, old liboqs uses sign_open rather than verify, and the internals change per scheme
    fprintf(stderr, "liboqs version not supported.\n");
    exit(1);
    #endif
    #endif
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
    size_t m_len, pk_len, sig_len;
    #ifdef OQS_VERSION_NUMBER
    #if OQS_VERSION_NUMBER == 0x0201811fL
    size_t length_signature = sig->length_sig_overhead;
    #endif
    #else
    size_t length_signature = sig->length_signature;
    #endif
    if (size < length_signature+sig->length_public_key) {
        if (size <= length_signature){
            sig_len = size;
            pk_len = 0;
        } else {
            sig_len = length_signature;
            pk_len = size - sig_len;
        }
        m_len = 0;
    } else {
        m_len = size - length_signature - sig->length_public_key;
        pk_len = sig->length_public_key;
        sig_len = length_signature;
    }
    u8 *message = calloc(m_len, 1);
    u8 *pk = calloc(pk_len, 1);
    u8 *signature = calloc(sig_len, 1);
    fread(pk, 1, pk_len, fp);
    fread(signature, 1, sig_len, fp);
    fread(message, 1, m_len, fp);
    fclose(fp);


    printf("pk:\t");
    print_buffer_hex(pk, pk_len);
    printf("pk_len:\t%zu\n", pk_len);
    printf("m:\t");
    print_buffer_hex(message, m_len);
    printf("m_len:\t%zu\n", m_len);
    printf("s:\t");
    print_buffer_hex(signature, sig_len);
    printf("sig_len:\t%zu\n", sig_len);

    free(signature);
    free(pk);
    free(message);
    OQS_SIG_free(sig);
}
