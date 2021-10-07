#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libtlv.h"

#define UNUSED(x) (void)(x)

typedef struct {
    int (*func)(void *);
    char *func_name;
} commands_s;

int main (int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    printf("Retorno:%d",init());
    return 0;
}
