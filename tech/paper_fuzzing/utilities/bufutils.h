#pragma once
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// io
void print_buffer_hex_to_buf(char *out_buffer, unsigned char *in_buffer, int len);
void print_buffer_hex(unsigned char *buffer, int len);
void print_buffer_hex_to_fp(FILE *fp, unsigned char *buffer, int len);
void print_buffer_bin_to_buf(char *out_buffer, unsigned char *in_buffer, int len);
void print_buffer_bin(unsigned char *buffer, int len);

// operations
void xor_buffers(unsigned char *out_buf, unsigned char *in_buf1, unsigned char *in_buf2, int len);
bool is_zero_buffer(unsigned char *buffer, int len);
bool are_equal_buffers(unsigned char *buffer1, unsigned char *buffer2, int len);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

template <typename T>
void print_buffer_bin_to_stream(T &out_stream, unsigned char *in_buffer, int len);

template <typename T>
void print_buffer_hex_to_stream(T &out_stream, unsigned char *in_buffer, int len);

#endif