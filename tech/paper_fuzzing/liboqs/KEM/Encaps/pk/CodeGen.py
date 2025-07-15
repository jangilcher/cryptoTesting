import os
if 'BASEDIR_LVL' not in os.environ:
    os.environ['BASEDIR_LVL'] = "../../../"

from serialize import unserialize


def codegen(pk, coins, mask):
    source = """

#include "api.h"
#include "rng.h"
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
    source += "\n    unsigned char coins[48] = {%s};\n" % ", ".   join(map(str, map(int, coins)))
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
    inplace_xor(pk, mask, MASK_LEN);

    // redo
    randombytes_init(coins, NULL, 256);
    res = crypto_kem_enc(c2, ss2, pk);

    assert(res == 0);
    printf("Encap(pk) succeded.\\n");

    // this one will capture either same ciphertext or different ciphertext encapsulating same key
    if (memcmp(ss1, ss2, CRYPTO_BYTES) == 0)
    {
        printf("Encap(pk).ss = Encap(maul(pk)).ss\\n");
    }

    return 0;
}
"""
    return source

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

    pk = obj["x_buf"][1:-1]
    pk_bits = bitarray()
    pk_bits.frombytes(pk)

    pkp = obj["xp_buf"][1:-1]
    pkp_bits = bitarray()
    pkp_bits.frombytes(pkp)

    mask_bits = pk_bits ^ pkp_bits
    mask = mask_bits.tobytes()

    coins = b"geninput" + (b"\x00" * 40)

    # print("sk", len(sk), sk)
    # print("c", c)
    # print("mask", list(map(int, mask)))
    # print("ss_len", ss_len)

    code = codegen(pk, coins, mask)

    with open(args.source_file, "w") as f:
        f.write(code)

