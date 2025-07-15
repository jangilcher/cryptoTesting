import os
if 'BASEDIR_LVL' not in os.environ:
    os.environ['BASEDIR_LVL'] = "../../../"

from serialize import unserialize

def codegen(pk, m, sig, sigp):
    source = """
#include "api.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main()
{"""
    source += "\nunsigned char pk[] = {%s};\n" % ", ".   join(map(str, map(int, pk)))
    source += "\nsize_t m_len = %d;\n" % len(m)
    source += "\nsize_t sig_len = %d;\n" % len(sig)
    source += "\nsize_t sigp_len = %d;\n" % len(sigp)
    source += "\nunsigned char sig[] = {%s};\n"  % ", ".   join(map(str, map(int, sig)))
    source += "\nunsigned char sigp[] = {%s};\n"  % ", ".   join(map(str, map(int, sigp)))
    source += "\nunsigned char m[] = {%s};\n"  % ", ".   join(map(str, map(int, m)))

    source += """
    int res;

    res = verify(m, m_len, sig, sig_len, pk);
    printf("Verify(m, sig, pk) succeded.\\n");

    // assert successful decapsulation
    assert(res == 0);

    assert(
        (memcmp(sig, sigp, sig_len) != 0)
        || (sig_len != sigp_len)
    );
    printf("sig != maul(sig)\\n");

    // decapsulate mauled ciphertext
    res = verify(m, m_len, sigp, sigp_len, pk);

    assert(res == 0);
    printf("Verify(m, maul(sig), pk) succeded.\\n");

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

    x = obj["x_buf"]
    sig_len = int.from_bytes(x[:8], "little")
    sig = x[9:9+sig_len]

    xp = obj["xp_buf"]
    sigp_len = int.from_bytes(xp[:8], "little")
    sigp = xp[9:9+sigp_len]

    m = obj["aux_list"][2][1]
    pk = obj["aux_list"][0][1]

    print("m", m, len(m))

    code = codegen(pk, m, sig, sigp)

    with open(args.source_file, "w") as f:
        f.write(code)
    
    # with open("fakerng.h", "w") as f:
    #     f.write(fakerng_h)
    
    # with open("fakerng.c", "w") as f:
    #     f.write(fakerng_c)
    

