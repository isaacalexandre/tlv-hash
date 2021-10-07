#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libtlv.h"

#define UNUSED(x) (void)(x)

typedef struct {
    int (*func)(void *);
    char *func_name;
} commands_s;

static int f_tlv_set(void* p_value)
{
    UNUSED(p_value);
    return 0;
}

static int f_tlv_get(void* p_value)
{
    UNUSED(p_value);
    return 0;
}

static int f_tlv_pretty(void* p_value)
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

    for (i = 0; i < u32_cmd_number; i++)
        printf("%4d. %-42s", i, commands[i].func_name);

    printf("Retorno:%d",init());
    return 0;
}
