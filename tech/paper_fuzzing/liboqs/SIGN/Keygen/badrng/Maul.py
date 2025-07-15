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

__machine_byte_order__ = "little"
__sizeof_size_t__ = 8

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
def Maul(x, fmt, sigma):
    x = bytes(x)
    assert(len(x) * 8 == fmt[0][0])
    assert(len(x) == 1)

    # print()
    # print("fmt:", fmt)
    # print("x:", x)
    # print("|x|:", len(x) * 8)

    if sigma == None:
        sigma = 0

    if sigma < 256**len(x):
        xp = int.to_bytes(sigma+1, 1, __machine_byte_order__)

    sigmap = sigma + 1
    one_more = bool(sigma + 1 < 256**len(x) - 1) # note we encode sigma+1
    lbl = DIFF
    expected_result = bool(lbl == EQ)

    # print("xp:", xp)
    # print(f"sigmap = {sigmap}")
    # print(f"one_more = {one_more}")
    # print(f"lbl =", "DIFF" if lbl == DIFF else "EQ")
    # print(f"expected_result = {expected_result}")
    # print()

    # input()

    return xp, sigmap, one_more, expected_result


__cur_step__ = 0
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

    # one_more = True
    # sigma = 250
    # while one_more:
    #     xp, sigmap, one_more, expres = Maul(x, fmt, sigma)
    #     sigma = sigmap
    # os.kill(os.getpid(), signal.SIGINT)
    # time.sleep(.25)
    # os.kill(os.getpid(), signal.SIGKILL)

    xp, sigmap, one_more, expres = Maul(x, fmt, sigma)
    # logging.debug(xp)
    if sigma % 100 == 0:
        logging.debug(f"{sigma} / {256 ** BufBytelen(fmt)}")
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
