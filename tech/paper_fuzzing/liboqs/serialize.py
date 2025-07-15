from cffi import FFI
from sympy import false
import os
BASEDIR_LVL = os.environ['BASEDIR_LVL']
# import sys
# from pathlib import Path
# sys.path.insert(1, str(Path(BASEDIR_LVL).resolve()))

__machine_byte_order__ = "little"


def sizeof_types():
    ffi = FFI()
    for header in ["../utilities/types.h", "../utilities/buf_list.h", "API.h", "../utilities/fmt_str_parser.h"]:
        with open(BASEDIR_LVL + header) as f:
            header_src = "".join(filter(lambda x: "#include" not in x, f.readlines()))
        ffi.cdef(header_src)

    sizes = {}
    for _t in ["len_t", "lbl_t", "aux_t", "pp_t", "fmt_t", "tuple_t", "rv_t", "out_t", "in_t", "exp_res_t"]:
        sizes[_t] = ffi.sizeof(_t)

    return sizes


__sizeof_types__ = sizeof_types()


def _unserialize_type(buf, _type, offset=None, raw=False):
    if not offset:
        offset = 0
    inc = __sizeof_types__[_type]
    val = buf[offset:offset+inc]
    if not raw:
        val = int.from_bytes(val, __machine_byte_order__)
    offset += inc
    return offset, val


def unserialize(buf):
    offset, pp_val = _unserialize_type(buf, "pp_t")
    offset, aux_list_len = _unserialize_type(buf, "len_t", offset)
    aux_list = []
    aux_buf_list = []
    # exit(1)
    for i in range(aux_list_len):
        _, aux_item_bytes = _unserialize_type(buf, "len_t", offset)
        aux_list_item_buf = buf[offset:offset+__sizeof_types__["len_t"]+aux_item_bytes]
        aux_item_buf = aux_list_item_buf[__sizeof_types__["len_t"]:__sizeof_types__["len_t"]+aux_item_bytes]
        offset += __sizeof_types__["len_t"]+aux_item_bytes
        tup = (aux_item_bytes, aux_item_buf)
        aux_list.append(tup)
        aux_buf_list.append(aux_list_item_buf)
    offset, fmt_list_len = _unserialize_type(buf, "len_t", offset)
    fmt_list = []
    fmt_buf_list = []
    for i in range(fmt_list_len):
        # due to binary alignment, structs may be longer than their ocntents
        offset, tup_buf = _unserialize_type(buf, "fmt_t", offset, raw=True)
        tup_offset, bitlen = _unserialize_type(tup_buf, "len_t")
        tup_offset, lbl = _unserialize_type(tup_buf, "lbl_t", tup_offset)
        tup = (bitlen, lbl)
        fmt_list.append(tup)
        fmt_buf_list.append(tup_buf)
    offset, x_bytes = _unserialize_type(buf, "len_t", offset)
    x_buf = buf[offset:offset+x_bytes]
    offset += x_bytes
    xp_buf = buf[offset:offset+x_bytes]
    offset += x_bytes
    offset, y_bytes = _unserialize_type(buf, "len_t", offset)
    y_buf = buf[offset:offset+y_bytes]
    offset += y_bytes
    offset, y_retval = _unserialize_type(buf, "rv_t", offset)
    offset, expres = _unserialize_type(buf, "exp_res_t", offset)

    obj = {}
    obj["pp_val"] = pp_val
    obj["aux_list_len"] = aux_list_len
    obj["aux_list"] = aux_list
    obj["aux_buf_list"] = aux_buf_list
    obj["fmt_list_len"] = fmt_list_len
    obj["fmt_list"] = fmt_list
    obj["fmt_buf_list"] = fmt_buf_list
    obj["x_bytes"] = x_bytes
    obj["x_buf"] = x_buf
    obj["xp_buf"] = xp_buf
    obj["y_bytes"] = y_bytes
    obj["y_buf"] = y_buf
    obj["y_retval"] = y_retval
    obj["expres"] = expres
    return obj


def serialize(pp, aux_list, fmt_buf, x, xp, y, y_retval, expres):
    """ NOTE: this assumes |x| = |xp|
    """
    buf = b""
    buf += int.to_bytes(pp, __sizeof_types__["pp_t"], __machine_byte_order__)
    buf += int.to_bytes(len(aux_list), __sizeof_types__["len_t"], __machine_byte_order__)
    for aux_item_bytes, aux_item_buf in aux_list:
        buf += int.to_bytes(aux_item_bytes, __sizeof_types__["len_t"], __machine_byte_order__)
        buf += aux_item_buf
    buf += int.to_bytes(len(fmt_buf), __sizeof_types__["len_t"], __machine_byte_order__)
    for tup_buf in fmt_buf:
        buf += tup_buf
    buf += int.to_bytes(len(x), __sizeof_types__["len_t"], __machine_byte_order__)
    buf += x
    buf += xp
    buf += int.to_bytes(len(y), __sizeof_types__["len_t"], __machine_byte_order__)
    buf += y
    buf += int.to_bytes(y_retval, __sizeof_types__["rv_t"], __machine_byte_order__)
    buf += bool.to_bytes(expres, __sizeof_types__["exp_res_t"], __machine_byte_order__)
    return buf


if __name__ == "__main__":
    print(__sizeof_types__)
    fn = "./fuzzinputs/test.bin"
    # fn = "./fuzzoutputs/default/crashes/id:000000,sig:06,src:000000,time:251,execs:529,op:Maul.so,pos:0"
    with open(fn, "rb") as f:
        buf = f.read()
    obj = unserialize(buf)
    for k in obj: print(k, obj[k])

    pp = obj["pp_val"]
    aux_list = obj["aux_list"]
    fmt_buf = obj["fmt_buf_list"]
    x = obj["x_buf"]
    xp = x
    y = obj["y_buf"]
    y_retval = obj["y_retval"]
    expres = bool(obj["expres"])
    _buf = serialize(pp, aux_list, fmt_buf, x, xp, y, y_retval, expres)
    assert(buf == _buf)
    print(buf == _buf)
