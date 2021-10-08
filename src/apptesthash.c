#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#include "libtlv.h"

#define UNUSED(x) (void)(x)
#define BUFFER_INPUT_SIZE 1024

typedef struct {
    int (*func)(uint8_t *);
    char *func_name;
} commands_s;

static int f_tlv_set(uint8_t * p_value)
{
    UNUSED(p_value);
    return 0;
}

static int f_tlv_get(uint8_t * p_value)
{
    UNUSED(p_value);
    return 0;
}

static int f_tlv_pretty(uint8_t * p_value)
{
    UNUSED(p_value);
    return 0;
}

commands_s commands[] = {
    {NULL, "Exit"},
    {f_tlv_set, "[SET] TLV Set - Adiciona uma ou mais tags"},
    {f_tlv_get, "[GET] TLV Get - Requisita uma ou mais tags"},
    {f_tlv_pretty, "[PRY] TLV Pretty - Requisita uma ou mais tags e exibe em forma arvore"},
};

int main (int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    uint32_t u32_cmd_number = (sizeof(commands) / sizeof(commands_s));
    uint32_t i = 0;
    uint32_t u32_option = 0;
    uint32_t u32_ret = 0;
    uint8_t u8_buffer[BUFFER_INPUT_SIZE];

    while(1) {
        for (i = 0; i < u32_cmd_number; i++)
            printf("%4d. %-42s\n", i, commands[i].func_name);

        u32_option = read_ulong();

        if (u32_option == 0) {
            printf("bye\n");
            return 0;
        } else if (u32_option < u32_cmd_number) {
            if (commands[u32_option].func != NULL) {
                read_string((char*)u8_buffer, BUFFER_INPUT_SIZE);
                u32_ret = commands[u32_option].func(u8_buffer);
                printf("ret = %s[%d]\n\n", commands[u32_option].func_name, u32_ret);
            }
        } else {
            printf("Invalid function... ");
        }
    }
    return 0;
}
