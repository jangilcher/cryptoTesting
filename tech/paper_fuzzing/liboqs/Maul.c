#include "API.h"
#include "../utilities/fmt_str_parser.h"
#include "../utilities/bufutils.h"
#include "serialize.h"

// You need to use -I /path/to/aflpp/include
// #include "../custom_mutator_helpers.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#define LOG(_fp, _fn, ...) do { (_fp) = fopen((_fn), "a+"); fprintf ((_fp), __VA_ARGS__); fclose((_fp)); } while (0)
#define MICROSECONDS_IN_A_SECOND 1e6

typedef struct {

} afl_t;

typedef struct my_mutator {

  afl_t *afl;

  // any additional data here!
  uint64_t cur_step;
  bool keep_running;

  // // Reused buffers:
  // BUF_VAR(u8, fuzz);

} my_mutator_t;



void Maul(in_t *xp, u64 *sigmap, bool *one_more, exp_res_t *expres,  // outputs
          in_t *x, fmt_t *fmt, u64 sigma) // inputs
{
  u64 bytelen = BufBytelen(fmt);
  assert(bytelen == x->bytes);
  xp->bytes = bytelen;

  lbl_t lbl;

  if (sigma == (u64)(-3)) {
    memcpy(xp->buf, x->buf, bytelen);
  }
  else if (sigma == (u64)(-2)) {
    memset(xp->buf, 0x00, xp->bytes);
  }
  else if (sigma == (u64)(-1)) {
    memset(xp->buf, 0xFF, xp->bytes);
  }
  else {
    // maul xp
    memcpy(xp->buf, x->buf, bytelen);
    u64 mask_byte = sigma/8;
    u64 mask_bit = sigma % 8;
    xp->buf[mask_byte] ^= (128 >> mask_bit);
  }

  // verifying bit manipulation
  // printf("sigma: %lu, ", sigma);
  // printf("mask_byte: %lu, ", mask_byte);
  // printf("mask_bit: %lu, ", mask_bit);
  // printf("mask byte: %x\n", (128 >> mask_bit));
  // print_buffer_hex(x->buf, x->bytes);
  // print_buffer_hex(xp->buf, xp->bytes);

  *sigmap = sigma + 1;
  if (sigma < (u64)(-3)) { // python says sigma >= 0, but here we are unsigned
    *one_more = (bool)(*sigmap < 8 * BufBytelen(fmt));
    lbl = GetLabel(sigma, fmt);
  } else {
    *one_more = true;
    lbl = (memcmp(xp->buf, x->buf, bytelen) == 0) ? EQ : DIFF;
  }
  *expres = (bool)(lbl == EQ);
}


/**
 * Initialize this custom mutator
 *
 * @param[in] afl a pointer to the internal state object. Can be ignored for
 * now.
 * @param[in] seed A seed for this mutator - the same seed should always mutate
 * in the same way.
 * @return Pointer to the data object this custom mutator instance should use.
 *         There may be multiple instances of this mutator in one afl-fuzz run!
 *         Return NULL on error.
 */
my_mutator_t *afl_custom_init(afl_t *afl, unsigned int seed) {

  srand(seed);  // needed also by surgical_havoc_mutate()

  my_mutator_t *data = calloc(1, sizeof(my_mutator_t));
  if (!data) {

    perror("afl_custom_init alloc");
    return NULL;

  }

  data->afl = afl;
  data->cur_step = -3;
  data->keep_running = true;

  // printf("DEBUG: Hello from init\n");

  return data;

}


/**
 * Perform custom mutations on a given input
 *
 * (Optional for now. Required in the future)
 *
 * @param[in] data pointer returned in afl_custom_init for this fuzz case
 * @param[in] buf Pointer to input data to be mutated
 * @param[in] buf_size Size of input data
 * @param[out] out_buf the buffer we will work on. we can reuse *buf. NULL on
 * error.
 * @param[in] add_buf Buffer containing the additional test case
 * @param[in] add_buf_size Size of the additional test case
 * @param[in] max_size Maximum size of the mutated output. The mutation must not
 *     produce data larger than max_size.
 * @return Size of the mutated output.
 */
size_t afl_custom_fuzz(my_mutator_t *data,
                       uint8_t *buf, size_t buf_size,
                       u8 **out_buf, 
                       uint8_t *add_buf, size_t add_buf_size, // add_buf can be NULL
                       size_t max_size)
{
  if (!data->keep_running)
  {
    
    // stop fuzzing
    // FILE *fp; LOG(fp, "log.maul", "DEBUG: Terminate fuzzing loop\n");
    printf("DEBUG: Terminate fuzzing loop.\n");

    kill(getpid(), SIGTERM);
    usleep((useconds_t)(.25 * MICROSECONDS_IN_A_SECOND)); // waiting seems to allow SIGKILL to arrive, sadly it does not work with SIGINT
    return 0; // this does make afl stop..
  }

  // FILE *fp; LOG(fp, "log.maul", "DEBUG: inside afl_custom_fuzz: %lu\n", data->cur_step);
  // printf("DEBUG: inside afl_custom_fuzz\n");

  u64 sigma = data->cur_step;

  pp_t pp;
  aux_t aux;
  fmt_t fmt;
  in_t x;
  in_t xp;
  out_t y;
  exp_res_t expres;

  // LOG(fp, "log.maul", "buf_size: %lu\n", (u64) buf_size);
  unserialize(buf, &pp, &aux, &fmt, &x, &xp, &y, &expres); // assuming buf_size bytes were serialised

  u64 sigmap;
  bool one_more;
  Maul(&xp, &sigmap, &one_more, &expres, &x, &fmt, sigma);

  if (sigma % 1000 == 0) {
    printf("DEBUG: %ld / %ld\n", sigma, 8 * BufBytelen(&fmt));
  }

  data->cur_step = sigmap;
  data->keep_running = one_more;

  // serialise mauled bits to out_buf
  u64 bytes = serialize(out_buf, &pp, &aux, &fmt, &x, &xp, &y, &expres);

  // free unserialized data structures
  free(x.buf);
  free(xp.buf);
  free(y.buf);
  free(fmt.list);
  buf_list_free(&aux);

  return bytes;
}

/**
 * Deinitialize everything
 *
 * @param data The data ptr from afl_custom_init
 */
void afl_custom_deinit(my_mutator_t *data)
{
  //   free(data->fuzz_buf);
  free(data);
}
