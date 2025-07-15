import os
if 'BASEDIR_LVL' not in os.environ:
    os.environ['BASEDIR_LVL'] = "../../../"

from serialize import unserialize


def codegen(pk, coins, mask):
    source = """

#include "api.h"
#include "fakerng.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void inplace_xor(unsigned char *buf, unsigned char *mask, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        buf[i] ^= mask[i];
    }
}

#define MASK_LEN %d

int main()
{""" % (len(mask))
    source += "\n    unsigned char pk[] = {%s};\n" % ", ".  join(map(str, map(int, pk)))
    source += "\n    unsigned char coins[] = {%s};\n" % ", ".   join(map(str, map(int, coins)))
    source += "\n    unsigned char mask[] = {%s};\n" % ", ".join(map(str, map(int, mask)))
    source += """
    unsigned char c1[CRYPTO_CIPHERTEXTBYTES];
    unsigned char c2[CRYPTO_CIPHERTEXTBYTES];
    unsigned char ss1[CRYPTO_BYTES];
    unsigned char ss2[CRYPTO_BYTES];
    int res;

    randombytes_init(coins, NULL, 256);

    res = crypto_kem_enc(c1, ss1, pk);

    // assert successful encapsulation
    assert(res == 0);

    // maul
    inplace_xor(coins, mask, MASK_LEN);

    // redo
    randombytes_init(coins, NULL, 256);
    res = crypto_kem_enc(c2, ss2, pk);

    assert(res == 0);
    printf("Encap(pk) succeded.\\n");

    // this one will capture either same ciphertext or different ciphertext encapsulating same key
    if (memcmp(ss1, ss2, CRYPTO_BYTES) == 0)
    {
        printf("Encap(pk, coins).ss = Encap(pk, maul(coins)).ss\\n");
    }

    return 0;
}
"""
    return source

fakerng_h = """#ifndef rng_h
#define rng_h

void
randombytes_init(unsigned char *entropy_input,
                 unsigned char *personalization_string,
                 int security_strength);

int
randombytes(unsigned char *x, unsigned long long xlen);

#else
#error Included fakerng.h and rng.h at the same time.
#endif
"""

fakerng_c = """#include "fakerng.h"

unsigned char seed;

void
randombytes_init(unsigned char *entropy_input,
                 unsigned char *personalization_string,
                 int security_strength)
{
    seed = entropy_input[0];
}

int
randombytes(unsigned char *x, unsigned long long xlen)
{
    for (unsigned long long i = 0; i < xlen; i++)
    {
        x[i] = seed;
    }

    return 0;
}
"""


from bitarray import bitarray

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("input_bin_file", help="input binary file name")
    parser.add_argument("source_file", help="source file name", default="example.c")
    args = parser.parse_args()

    with open(args.input_bin_file, "rb") as f:
        bin = f.read()
    obj = unserialize(bin)
    for k in obj: print(k)

    pk = obj["aux_list"][0][1]

    x = obj["x_buf"]
    x_bits = bitarray()
    x_bits.frombytes(x)

    xp = obj["xp_buf"]
    xp_bits = bitarray()
    xp_bits.frombytes(xp)

    mask_bits = x_bits ^ xp_bits
    mask = mask_bits.tobytes()

    # print("x", len(x), x)
    # print("c", c)
    # print("mask", list(map(int, mask)))
    # print("ss_len", ss_len)

    code = codegen(pk, x, mask)

    with open(args.source_file, "w") as f:
        f.write(code)
    
    with open("fakerng.h", "w") as f:
        f.write(fakerng_h)
    
    with open("fakerng.c", "w") as f:
        f.write(fakerng_c)
    

