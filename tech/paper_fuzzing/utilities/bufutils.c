// this library supports both C and C++
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#ifdef __cplusplus
#include <iostream>
#include <fstream>
#endif
#include "bufutils.h"

char print_buffer_hex_dict[] = {'0', '1', '2', '3', '4',
                                '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

/**
 * @brief Prints unsigned char buffer as a hex string to output NULL terminated char buffer
 * 
 * @param out_buffer Ouput NULL terminated char buffer of length 2 * len + 1
 * @param in_buffer Input unsigned char buffer to print
 * @param len input Buffer bytelength
 */
void print_buffer_hex_to_buf(char *out_buffer, unsigned char *in_buffer, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        out_buffer[2 * i] = print_buffer_hex_dict[in_buffer[i] >> 4];
        out_buffer[2 * i + 1] = print_buffer_hex_dict[in_buffer[i] & 15];
        // snprintf(&out_buffer[2*i], 2, "%02x", in_buffer[i]);
    }
    out_buffer[2 * len] = '\0';
}

/**
 * @brief Prints unsigned char buffer as a hex string to stdout
 * 
 * @param buffer Input unsigned char buffer to print
 * @param len Input buffer bytelength
 */
void print_buffer_hex(unsigned char *buffer, int len)
{
    char *out_buffer = (char *)malloc((2 * len + 1) * sizeof(char));
    print_buffer_hex_to_buf(out_buffer, buffer, len);
    printf("%s\n", out_buffer);
    free(out_buffer);
}

/**
 * @brief Prints unsigned char buffer as a hex string to a file
 * 
 * @param buffer Input unsigned char buffer to print
 * @param len Input buffer bytelength
 * @param fp Output file pointer (should already be open!)
 */
void print_buffer_hex_to_fp(FILE *fp, unsigned char *buffer, int len)
{
    char *out_buffer = (char *)malloc((2 * len + 1) * sizeof(char));
    print_buffer_hex_to_buf(out_buffer, buffer, len);
    fprintf(fp, "%s\n", out_buffer);
    free(out_buffer);
}

/**
 * @brief Prints unsigned char buffer as a binary string to output NULL terminated char buffer
 * 
 * @param out_buffer Ouput NULL terminated char buffer of length 8 * len + 1
 * @param in_buffer Input unsigned char buffer to print
 * @param len input Buffer bytelength
 */
void print_buffer_bin_to_buf(char *out_buffer, unsigned char *in_buffer, int len)
{
    int i, j;
    unsigned char mask;
    for (i = 0; i < len; i++)
    {
        mask = 1;
        for (j = 0; j < 8; j++)
        {
            out_buffer[i * 8 + j] = '0' + (int)((in_buffer[i] & mask) > 0);
            mask <<= 1;
        }
    }
    out_buffer[8 * len] = '\0';
}

/**
 * @brief Prints unsigned char buffer as a binary string to stdout
 * 
 * @param buffer Input unsigned char buffer to print
 * @param len Input buffer bytelength
 */
void print_buffer_bin(unsigned char *buffer, int len)
{
    char *out_buffer = (char *)malloc((8 * len + 1) * sizeof(char));
    print_buffer_bin_to_buf(out_buffer, buffer, len);
    printf("%s\n", out_buffer);
    free(out_buffer);
}

/**
 * @brief Bytewise XORs two unsigned char buffers of bytelength len
 * 
 * @param out_buf Output resulting buffer
 * @param in_buf1 First input buffer
 * @param in_buf2 Second input byffer
 * @param len Input buffers' bytelength
 */
void xor_buffers(unsigned char *out_buf, unsigned char *in_buf1, unsigned char *in_buf2, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        out_buf[i] = in_buf1[i] ^ in_buf2[i];
    }
}

/**
 * @brief Checks if input buffer is the 0 buffer
 * 
 * @param buffer Input buffer
 * @param len Input buffer's bytelength
 * @return true 
 * @return false 
 */
bool is_zero_buffer(unsigned char *buffer, int len)
{
    unsigned char zero;
    int i;
    zero = 0;
    for (i = 0; i < len; i++)
    {
        zero |= buffer[i];
    }
    return zero == 0;
}

/**
 * @brief Checks if input buffers are equal
 * 
 * @param buffer1 First input buffer
 * @param buffer2 Second input buffer
 * @param len Input buffers' bytelength
 * @return true 
 * @return false 
 */
bool are_equal_buffers(unsigned char *buffer1, unsigned char *buffer2, int len)
{
    int i;
    bool equal;
    equal = true;
    for (i = 0; i < len; i++)
    {
        equal &= (buffer1[i] == buffer2[i]);
    }
    return equal;
}

#ifdef __cplusplus

/**
 * @brief Prints buffer to output C++ stream as a binary string
 * 
 * @tparam T Output stream type (supported: file, io)
 * @param out_stream C++ output stream
 * @param in_buffer Input buffer
 * @param len Input buffer's bytelength
 */
template <typename T>
void print_buffer_bin_to_stream(T &out_stream, unsigned char *in_buffer, int len)
{
    char *out_buffer = (char *)malloc(8 * len + 1);
    print_buffer_bin_to_buf(out_buffer, in_buffer, len);
    out_stream << out_buffer;
    free(out_buffer);
};

/**
 * @brief Prints buffer to output C++ stream as a hex string
 * 
 * @tparam T Output stream type (supported: file, io)
 * @param out_stream C++ output stream
 * @param in_buffer Input buffer
 * @param len Input buffer's bytelength
 */
template <typename T>
void print_buffer_hex_to_stream(T &out_stream, unsigned char *in_buffer, int len)
{
    char *out_buffer = (char *)malloc(2 * len + 1);
    print_buffer_hex_to_buf(out_buffer, in_buffer, len);
    out_stream << out_buffer;
    free(out_buffer);
}

// explicitly instantiate types to avoid undefined reference
template void print_buffer_bin_to_stream<std::ostream>(std::ostream &, unsigned char *, int);
template void print_buffer_hex_to_stream<std::ostream>(std::ostream &, unsigned char *, int);
template void print_buffer_bin_to_stream<std::fstream>(std::fstream &, unsigned char *, int);
template void print_buffer_hex_to_stream<std::fstream>(std::fstream &, unsigned char *, int);
template void print_buffer_bin_to_stream<std::ofstream>(std::ofstream &, unsigned char *, int);
template void print_buffer_hex_to_stream<std::ofstream>(std::ofstream &, unsigned char *, int);

#endif