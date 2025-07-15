import random
import os, signal
import time
import logging
from cffi import FFI

# importing the fmt_str_parser and colors module
import sys
from pathlib import Path
BASEDIR_LVL = os.environ['BASEDIR_LVL']
sys.path.insert(1, str(Path(BASEDIR_LVL).resolve()))
from serialize import serialize, unserialize
sys.path.insert(1, str(Path(BASEDIR_LVL + '../utilities').resolve()))
from fmt_str_parser import BufBytelen, BufBitlen, GetLabel # type: ignore
from colors import Colors # type: ignore
logging.basicConfig(format=f'{Colors.Yellow}%(levelname)s: %(message)s', level=logging.DEBUG)


def load_lbl_t_enum():
    # load lbl_t labels
    ffi = FFI()
    with open(BASEDIR_LVL + "../utilities/fmt_str_parser.h") as f:
        header_src = "".join(filter(lambda x: "#include" not in x, f.readlines()))
        ffi.cdef(header_src)
    c = ffi.dlopen('c')
    EQ = c.EQ
    DIFF = c.DIFF
    return EQ, DIFF
EQ, DIFF = load_lbl_t_enum()


from bitarray import bitarray


def cmp_strings_valid_section(bs1, bs2, fmt):
    assert(len(bs1) == len(bs2))
    for i in range(len(bs1)):
        lbl = GetLabel(i, fmt)
        if lbl == DIFF:
            if bs1[i] != bs2[i]:
                return DIFF
    return EQ


def Maul(x, fmt, sigma):
    x = bytes(x)
    x_bits = bitarray(0)
    x_bits.frombytes(x)

    bytelen = BufBytelen(fmt)

    if sigma == None:
        assert(False)
        sigma = 0

    if sigma == -3:
        xp = bitarray(0)
        xp.frombytes(x)
        lbl = EQ  # x = x'
        xp = xp.tobytes()
    elif sigma == -2:
        xp = bitarray('0' * 8 * bytelen)
        lbl = cmp_strings_valid_section(x_bits, xp, fmt)
        xp = xp.tobytes()
    elif sigma == -1:
        xp = bitarray('1' * 8 * bytelen)
        lbl = cmp_strings_valid_section(x_bits, xp, fmt)
        xp = xp.tobytes()
    else:
        mask = bitarray('0' * 8 * bytelen)
        mask[sigma] = 1

        xp = (mask ^ x_bits).tobytes()

    sigmap = sigma + 1

    one_more = bool(sigmap < 8 * bytelen)

    if sigma >= 0:
        lbl = GetLabel(sigma, fmt)

    expected_result = bool(lbl == EQ)

    return xp, sigmap, one_more, expected_result


def unit_tests():
    # the code below should develop all possible flips

    fmt = [(8, EQ), (6, DIFF), (2, EQ), (8, EQ)]
    x = b"\xFF" * BufBytelen(fmt)

    one_more = True
    sigma = None
    out = ""
    while one_more:
        xp, sigma, one_more, exp_res = Maul(x, fmt, sigma)
        out += f"{xp} {sigma} {one_more} {exp_res}\n"
    print(out)

    expected = "b'\\x7f\\xff\\xff' 1 True True\n"
    expected += "b'\\xbf\\xff\\xff' 2 True True\n"
    expected += "b'\\xdf\\xff\\xff' 3 True True\n"
    expected += "b'\\xef\\xff\\xff' 4 True True\n"
    expected += "b'\\xf7\\xff\\xff' 5 True True\n"
    expected += "b'\\xfb\\xff\\xff' 6 True True\n"
    expected += "b'\\xfd\\xff\\xff' 7 True True\n"
    expected += "b'\\xfe\\xff\\xff' 8 True True\n"
    expected += "b'\\xff\\x7f\\xff' 9 True False\n"
    expected += "b'\\xff\\xbf\\xff' 10 True False\n"
    expected += "b'\\xff\\xdf\\xff' 11 True False\n"
    expected += "b'\\xff\\xef\\xff' 12 True False\n"
    expected += "b'\\xff\\xf7\\xff' 13 True False\n"
    expected += "b'\\xff\\xfb\\xff' 14 True False\n"
    expected += "b'\\xff\\xfd\\xff' 15 True True\n"
    expected += "b'\\xff\\xfe\\xff' 16 True True\n"
    expected += "b'\\xff\\xff\\x7f' 17 True True\n"
    expected += "b'\\xff\\xff\\xbf' 18 True True\n"
    expected += "b'\\xff\\xff\\xdf' 19 True True\n"
    expected += "b'\\xff\\xff\\xef' 20 True True\n"
    expected += "b'\\xff\\xff\\xf7' 21 True True\n"
    expected += "b'\\xff\\xff\\xfb' 22 True True\n"
    expected += "b'\\xff\\xff\\xfd' 23 True True\n"
    expected += "b'\\xff\\xff\\xfe' 24 False True\n"

    if out == expected:
        print("Tests passed")
    else:
        print("Test failed")


if __name__ == "__main__":
    unit_tests()


__cur_step__ = -3
__keep_running__ = True
def init(seed):
    random.seed(seed)
    logging.debug("Hello from init")


def fuzz(buf, add_buf, max_size):
    """
    Called per fuzzing iteration.

    @type buf: bytearray
    @param buf: The buffer that should be mutated.

    @type add_buf: bytearray
    @param add_buf: A second buffer that can be used as mutation source.

    @type max_size: int
    @param max_size: Maximum size of the mutated output. The mutation must not
        produce data larger than max_size.

    @rtype: bytearray
    @return: A new bytearray containing the mutated data
    """
    global __cur_step__
    global __keep_running__

    if not __keep_running__:
        # stopp fuzzing
        logging.debug("Terminating fuzzing loop.")
        os.kill(os.getpid(), signal.SIGINT)
        time.sleep(.25)
        os.kill(os.getpid(), signal.SIGKILL)

    sigma = __cur_step__
    obj = unserialize(buf)

    x = obj["x_buf"]
    fmt = obj["fmt_list"]
    xp, sigmap, one_more, expres = Maul(x, fmt, sigma)
    # logging.debug(xp)
    if sigma % 1000 == 0:
        logging.debug(f"{sigma} / {8 * BufBytelen(fmt)}")
    __cur_step__ = sigmap
    __keep_running__ = one_more

    pp = obj["pp_val"]
    aux_list = obj["aux_list"]
    fmt_buf = obj["fmt_buf_list"]
    y = obj["y_buf"]
    y_retval = obj["y_retval"]
    out = serialize(pp, aux_list, fmt_buf, x, xp, y, y_retval, expres)
    return bytearray(out)

# optional for Python, but generates a warning if missing
def deinit():
    logging.debug("Hello from deinit")
