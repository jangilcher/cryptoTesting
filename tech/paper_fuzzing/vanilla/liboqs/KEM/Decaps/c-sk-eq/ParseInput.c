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
    OQS_KEM *kem = NULL;
    const char *kem_name = OQS_KEM_alg_identifier(ALG_ID);
    kem = OQS_KEM_new(kem_name);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    size_t sk_len, ct_len;
    if (size < kem->length_secret_key + kem->length_ciphertext){
        if (size <= kem->length_secret_key) {
            sk_len = size;
            ct_len = 0;
        } else {
            sk_len = kem->length_secret_key;
            ct_len = size - sk_len;
        }
    } else {
            sk_len = kem->length_secret_key;
            ct_len = kem->length_ciphertext;
    }
    u8 *shared_secret_d = calloc(kem->length_shared_secret, 1);
    u8 *ciphertext = calloc(ct_len, 1);
    u8 *sk = calloc(sk_len, 1);
    fread(sk, 1, sk_len, fp);
    fread(ciphertext, 1, ct_len, fp);
    fclose(fp);

    printf("sk:\t");
    print_buffer_hex(sk, sk_len);
    printf("m_len:\t%zu\n", sk_len);

    printf("ct:\t");
    print_buffer_hex(ciphertext, ct_len);
    printf("m_len:\t%zu\n", ct_len);

    OQS_KEM_free(kem);
    free(sk);
    free(ciphertext);
    free(shared_secret_d);
}
