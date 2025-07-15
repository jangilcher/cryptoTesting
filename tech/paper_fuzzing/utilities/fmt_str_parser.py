EQ = "eq"
DIFF = "diff"

def bits_to_bytes(bits):
    return (bits + 7)//8

def BufBitlen(fmt):
    bitlen = 0
    for (ell, lbl) in fmt:
        bitlen += ell
    return bitlen

def BufBytelen(fmt):
    return bits_to_bytes(BufBitlen(fmt))

def GetLabel(i, fmt):
    assert(len(fmt) != 0)
    cnt = -1
    idx = -1
    while cnt < i:
        idx += 1
        (ell, lbl) = fmt[idx]
        cnt += ell

    # # equivalent code use in C, except for the extra bits handling at the end of a (ell, lbl) tuple
    # idx += 1
    # assert(idx < len(fmt))
    # (ell, lbl) = fmt[idx]
    # cnt += ell
    # while cnt < i:
    #     idx += 1
    #     assert(idx < len(fmt))
    #     (ell, lbl) = fmt[idx]
    #     cnt += ell

    return lbl

def unit_tests():

    fmt = [(8, EQ), (6, DIFF), (2, EQ), (8, EQ)]
    empty = []

    if BufBitlen(fmt) != 24:
        print("error, BufBitlen(fmt) != 24")

    if BufBitlen(empty) != 0:
        print("error, BufBitlen(empty) != 0")

    if BufBytelen(fmt) != 3:
        print("error, BufBytelen(fmt) != 3")

    if BufBytelen(empty) != 0:
        print("error, BufBytelen(empty) != 0")

    if GetLabel(0, fmt) != EQ:
        print("error, GetLabel(0, fmt) != EQ")

    if GetLabel(0, fmt) == DIFF:
        print("error, GetLabel(0, fmt) == DIFF")

    if GetLabel(1, fmt) != EQ:
        print("error, GetLabel(1, fmt) != EQ")

    if GetLabel(1, fmt) == DIFF:
        print("error, GetLabel(1, fmt) == DIFF")

    if GetLabel(7, fmt) != EQ:
        print("error, GetLabel(7, fmt) != EQ")

    if GetLabel(7, fmt) == DIFF:
        print("error, GetLabel(7, fmt) == DIFF")

    if GetLabel(8, fmt) == EQ:
        print("error, GetLabel(8, fmt) == EQ")

    if GetLabel(8, fmt) != DIFF:
        print("error, GetLabel(8, fmt) != DIFF")

    if GetLabel(13, fmt) == EQ:
        print("error, GetLabel(13, fmt) == EQ")

    if GetLabel(13, fmt) != DIFF:
        print("error, GetLabel(13, fmt) != DIFF")

    if GetLabel(14, fmt) != EQ:
        print("error, GetLabel(14, fmt) != EQ")

    if GetLabel(14, fmt) == DIFF:
        print("error, GetLabel(14, fmt) == DIFF")

    if GetLabel(15, fmt) != EQ:
        print("error, GetLabel(15, fmt) != EQ")

    if GetLabel(15, fmt) == DIFF:
        print("error, GetLabel(15, fmt) == DIFF")

    if GetLabel(BufBitlen(fmt)-1, fmt) != EQ:
        print("error, GetLabel(BufBitlen(&fmt)-1, fmt) != EQ")

    if GetLabel(BufBitlen(fmt)-1, fmt) == DIFF:
        print("error, GetLabel(BufBitlen(&fmt)-1, fmt) == DIFF")

    # if GetLabel(BufBitlen(fmt), fmt) != EQ: # this should fail with assertion error
    #     print("error, GetLabel(BufBitlen(&fmt), fmt) != EQ")

    # if GetLabel(BufBitlen(fmt), fmt) == DIFF: # this should fail with assertion error
    #     print("error, GetLabel(BufBitlen(&fmt), fmt) == DIFF")

    # if GetLabel(150, fmt) != EQ: # this should fail with assertion error
    #     print("error, GetLabel(150, fmt) != EQ")

    # if GetLabel(150, fmt) == DIFF: # this should fail with assertion error
    #     print("error, GetLabel(150, fmt) == DIFF")

    # if GetLabel(0, empty) != 'eq': # this should fail with Assertion error
    #     print("error, GetLabel(0, empty) != EQ")

    print("All tests run.")

if __name__ == "__main__":
    unit_tests()
