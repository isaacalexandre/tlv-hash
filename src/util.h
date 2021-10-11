#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FILE_WRITE,
    FILE_READ,
} file_io;

void read_string(char *buf, size_t n);
void input_ASCII_compensation(char *message);
unsigned long int read_ulong(void);
void convert_ascii_to_hex(unsigned char *buf_hex,
              int *size_buf_hex,
              char *buf_char,
              int size_buf_char);
void convert_hex_to_ascii(char *buf_char,
              int *size_buf_char,
              unsigned char *buf_hex,
              int size_buf_hex);
int file_read_size(char *path, long *file_size);
int file_io_chunk(file_io io, char *path, long offset,
              size_t chunk_size, char *buffer);
void remove_spaces(const char *input, char *result);
void remove_char(char *s, char c);

uint32_t convert_hex_to_u32(uint8_t *buf_hex, int size_buf_hex);


uint16_t crc16(uint8_t *buffer, size_t len);

#define MSB(x) ((uint8_t) ( x >> 8 ))
#define LSB(x) ((uint8_t) ( x & 0x00FF ))

#define GET16(buffer, index) ((uint16_t)(buffer[index]*256 + buffer[index+1]))

#define SET16(buffer, value)    do { \
                                    *(buffer) = (value >> 8) & 0xFF; \
                                    *(buffer + 1) = value & 0xFF; \
                                } while (0);
#define SET32(buffer, value)    do { \
                                    *(buffer) = (value >> 24) & 0xFF; \
                                    *(buffer + 1) = (value >> 16) & 0xFF; \
                                    *(buffer + 2) = (value >> 8) & 0xFF; \
                                    *(buffer + 3) = value & 0xFF; \
                                } while (0);


#ifdef __cplusplus
}
#endif

#endif
