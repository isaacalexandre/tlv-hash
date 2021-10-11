#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "util.h"

void read_string(char *buf, size_t n)
{
    size_t ix = 0;
    char ch;

    fseek(stdin, 0, SEEK_END);

    while (ix < n - 1) {
        ch = getchar();
        if (ch == '\r' || ch == '\n') {
            break;
        }
        buf[ix++] = ch;
    }

    buf[ix++] = '\0';
    input_ASCII_compensation(buf);
}

unsigned long int read_ulong(void)
{
    char buf[15];
    read_string(buf, sizeof(buf));
    return strtoul(buf, NULL, 10);
}

void convert_ascii_to_hex(unsigned char *buf_hex,
              int *size_buf_hex,
              char *buf_char,
              int size_buf_char)
{
    int i = 0, j = 0;

    for (i = 0, j = 0; i < size_buf_char / 2; i++, j += 2) {
        buf_char[j] = toupper(buf_char[j]);
        buf_hex[i] = buf_char[j] >= 'A' ? (buf_char[j] - 55) << 4 :
            (buf_char[j] - 48) << 4;

        buf_char[j+1] = toupper(buf_char[j+1]);
        buf_hex[i] |= buf_char[j+1] >= 'A' ? (buf_char[j+1] - 55) :
            (buf_char[j+1] - 48);
    }
    if (size_buf_hex)
        *size_buf_hex = i;
}

void convert_hex_to_ascii(char *buf_char,
              int *size_buf_char,
              unsigned char *buf_hex,
              int size_buf_hex)
{
    int i, j;
    char temp;

    memset(buf_char, 0, size_buf_hex * 2 + 1);
    for (i = 0, j = 0; i < size_buf_hex; i++, j += 2) {
        temp = (buf_hex[i] >> 4) & 0x0F;
        buf_char[j] = temp >= 0x0A ?  temp + 55 : temp + 48;

        temp = buf_hex[i] & 0x0F;
        buf_char[j+1] = temp >= 0x0A ?  temp + 55 : temp + 48;
    }
    if (size_buf_char)
        *size_buf_char = j;
}

/* file functions */
int file_read_size(char *path, long *file_size)
{
    FILE *fp;
    int ret;
    long size;

    fp = fopen(path, "r");
    if (fp == NULL)
        return -1;

    ret = fseek(fp, 0 , SEEK_END);
    if (ret < 0) {
        fclose(fp);
        return -1;
    }

    size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return -1;
    }

    ret = fclose(fp);
    if (ret < 0) {
        return -1;
    }

    *file_size = size;
    return 0;
}

int file_io_chunk(file_io io, char *path, long offset, size_t chunk_size, char *buffer)
{
    FILE *fp;
    int ret;
    size_t bytes;

    if (io == FILE_READ)
        fp = fopen(path, "rb");
    else
        fp = fopen(path, "a+");

    if (fp == NULL)
        return -1;

    ret = fseek(fp, offset , SEEK_SET);
    if (ret < 0) {
        fclose(fp);
        return -1;
    }

    if (io == FILE_READ)
        bytes = fread(buffer, chunk_size, 1, fp);
    else
        bytes = fwrite(buffer, chunk_size, 1, fp);
    if (!bytes) {
        fclose(fp);
        return -1;
    }

    ret = fclose(fp);
    if (ret < 0)
        return -1;

    return 0;
}

void remove_spaces(const char *input, char *result)
{
    int i, j = 0;

    for (i = 0; input[i] != '\0'; i++) {
        if (!isspace((unsigned char) input[i])) {
            result[j++] = input[i];
        }
    }

    result[j] = '\0';
}

void remove_char(char *s, char c)
{
    int dst = 0, src = 0;

    while (s[src]) {
        if (s[src] != c)
            s[dst++] = s[src];
        src++;
    }
    s[dst]=0;
}

void input_ASCII_compensation(char *message)
{
    int search_matriz = 0;
    int search_message = 0;
    int message_size = strlen(message);

    char table_conversion_ASCII [128][2] = {
            {-128,0xc7}, {-127,0xfc}, {-126,0xe9}, {-125,0xe2}, {-124,0xe4}, {-123,0xe0}, {-122,0xe5}, {-121,0xe7}, {-120,0xea}, {-119,0xeb},
            {-118,0xe8}, {-117,0xef}, {-116,0xee}, {-115,0xec}, {-114,0xc4}, {-113,0xc5}, {-112,0xc9}, {-111,0xe6}, {-110,0xc6}, {-109,0xf4},
            {-108,0xf6}, {-107,0xf2}, {-106,0xfb}, {-105,0xf9}, {-104,0xff}, {-103,0xd6}, {-102,0xdc}, {-101,0xf8}, {-100,0xa3}, { -99,0xd8},
            { -98,0xd7}, { -97, 0  }, { -96,0xe1}, { -95,0xed}, { -94,0xf3}, { -93,0xfa}, { -92,0xf1}, { -91,0xd1}, { -90,0xaa}, { -89,0xba},
            { -88,0xbf}, { -87,0xae}, { -86,0xac}, { -85,0xbd}, { -84,0xbc}, { -83,0xa1}, { -82,0xab}, { -81,0xbb}, { -80,0x5f}, { -79,0x5f},
            { -78,0x5f}, { -77,0xa6}, { -76,0xa6}, { -75,0xc1}, { -74,0xc2}, { -73,0xc0}, { -72,0xa9}, { -71,0xa6}, { -70,0xa6}, { -69,0x2b},
            { -68,0x2b}, { -67,0xa2}, { -66,0xa5}, { -65,0x2b}, { -64,0x2b}, { -63,0x2d}, { -62,0x2d}, { -61,0x2b}, { -60,0x2d}, { -59,0x2b},
            { -58,0xe3}, { -57,0xc3}, { -56,0x2b}, { -55,0x2b}, { -54,0x2d}, { -53,0x2d}, { -52,0xa6}, { -51,0x2d}, { -50,0x2b}, { -49,0xa4},
            { -48,0xf0}, { -47,0xd0}, { -46,0xca}, { -45,0xcb}, { -44,0xc8}, { -43, 0  }, { -42,0xcd}, { -41,0xce}, { -40,0xcf}, { -39,0x2b},
            { -38,0x2b}, { -37,0x5f}, { -36,0x5f}, { -35,0xa6}, { -34,0xcc}, { -33,0x5f}, { -32,0xd3}, { -31,0xdf}, { -30,0xd4}, { -29,0xd2},
            { -28,0xd5}, { -27,0xd5}, { -26,0xb5}, { -25,0xde}, { -24,0xde}, { -23,0xda}, { -22,0xdb}, { -21,0xd9}, { -20,0xfd}, { -19,0xdd},
            { -18,0xaf}, { -17,0xb4}, { -16, 0  }, { -15,0xb1}, { -14,0x5f}, { -13,0xbe}, { -12,0xb6}, { -11,0xa7}, { -10,0xf7}, {  -9,0xb8},
            {  -8,0xb0}, {  -7,0xa8}, {  -6,0xb7}, {  -5,0xb9}, {  -4,0xb3}, {  -3,0xb2}, {  -2,0x5f}, {  -1, 0  }
     };

    for(search_message = 0; search_message < message_size; search_message++){
        if((message[search_message] < 0)){
            for (search_matriz = 0; search_matriz < 128; search_matriz++) {
                if (message[search_message] == table_conversion_ASCII [search_matriz][0]) {
                    message[search_message] = table_conversion_ASCII [search_matriz][1];
                    search_matriz = 128;
                    //break;
                }
            }
        }
    }
}

/*
*   x^16 + x^12 + x^5 + x^0
*/
uint16_t crc16(uint8_t *buffer, size_t len)
{
    const uint16_t CRC_MASK = 0x1021;
    uint16_t data, crc = 0;
    size_t i;

    for ( ;len > 0; len--, buffer++) {
        data = (uint16_t) (((uint16_t) *buffer) << 8);
        for (i = 0; i < 8; i++, data <<= 1) {
            if ((crc ^ data) & 0x8000)
                crc = (uint16_t) ((crc << 1) ^ CRC_MASK);
            else
                crc <<= 1;
        }
    }

    return crc;
}


uint32_t convert_hex_to_u32(uint8_t *buf_hex, int size_buf_hex)
{
    uint32_t u32_aux = 0;
    int16_t i = 0;
    int16_t count = 0;
    if (size_buf_hex > 4)
        return 0;

    for(i = (size_buf_hex-1); i >= 0; i--)
        u32_aux = (u32_aux | (uint32_t)(buf_hex[count++]<<(i*8) ));
    return u32_aux;
}

