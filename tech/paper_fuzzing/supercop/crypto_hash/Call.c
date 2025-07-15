// TODO: include the (supercop) hash header file
// #include "jh_ref/jh_ref.h"
#include "crypto_hash.h"
typedef int ret_t;

#include "Call.h"
#include <stdlib.h>

void Call(out_t *out, in_t *in, pp_t *pp, fmt_t *fmt)
{
    out->bytes = bits_to_bytes(pp->out_bitlen);
    out->buf = (u8 *)calloc(out->bytes, sizeof(u8));
    // NOTE: Hashing in->buf is not enough! need to use fmt_t, and point to the initial byte of intended input
    //       For this reason, we instead define `offset` and then hash &(in->buf[offset]).
    //       Making this primitive-dependent eases the need for super complicated abstract handling of the format, etc
    u64 offset = bits_to_bytes(fmt->list[0].bitlen);
    u64 input_bytelen = bits_to_bytes(fmt->list[1].bitlen);
    // ret_t retval = Hash(pp->out_bitlen, &(in->buf[offset]), input_bitlen, out->buf);
    ret_t retval = crypto_hash(out->buf, &(in->buf[offset]), input_bytelen);
    out->retval = retval; // NOTE: header-only library means we end up with casting happening here
}