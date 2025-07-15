import os
if 'BASEDIR_LVL' not in os.environ:
    os.environ['BASEDIR_LVL'] = "../../../"

from serialize import unserialize


def codegen(sk, c, mask, ss_len):
    source = """
#include "api.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void _xor(unsigned char *out, unsigned char *buf, unsigned char *mask, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        out[i] = buf[i] ^ mask[i];
    }
}

#define SS_LEN %d
#define MASK_LEN %d

int main()
{""" % (ss_len, len(mask))
    source += "\n    unsigned char sk[] = {%s};\n" % ", ".  join(map(str, map(int, sk)))
    source += "\n    unsigned char c[] = {%s};\n" % ", ".   join(map(str, map(int, c)))
    source += "\n    unsigned char cp[CRYPTO_CIPHERTEXTBYTES];\n"
    source += "\n    unsigned char mask[] = {%s};\n" % ", ".join(map(str, map(int, mask)))
    source += """
    unsigned char ss1[SS_LEN];
    unsigned char ss2[SS_LEN];
    int res;

    res = crypto_kem_dec(ss1, c, sk);

    // assert successful decapsulation
    assert(res == 0);

    // maul ciphertext
    _xor(cp, c, mask, MASK_LEN);
    assert(memcmp(cp, c, MASK_LEN) != 0);
    printf("c != maul(c)\\n");

    // decapsulate mauled ciphertext
    res = crypto_kem_dec(ss2, cp, sk);

    // this decapsulation should fail
    assert(res == 0);
    printf("Decap(maul(c), sk) succeded.\\n");

    if (memcmp(ss1, ss2, SS_LEN) == 0)
    {
        printf("Decap(c, sk) = Decap(maul(c), sk)\\n");
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

    sk = obj["aux_list"][1][1]
    
    c = obj["x_buf"][1:-1]
    c_bits = bitarray()
    c_bits.frombytes(c)

    cp = obj["xp_buf"][1:-1]
    cp_bits = bitarray()
    cp_bits.frombytes(cp)

    mask_bits = c_bits ^ cp_bits
    mask = mask_bits.tobytes()

    ss_len = obj["y_bytes"]

    # print("sk", len(sk), sk)
    # print("c", c)
    # print("mask", list(map(int, mask)))
    # print("ss_len", ss_len)

    code = codegen(sk, c, mask, ss_len)
    # print(code)

    with open(args.source_file, "w") as f:
        f.write(code)

