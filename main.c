#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "Functions.h"
#include "commands.h"
#include "estructuras.h"
int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        ugit_err("Aprende a escribir\n");
        return 1;
    }
    if(strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
    {
        printf("Uso: %s <nombre_del_archivo>\n", argv[0]);
        printf("Calcula el hash de Jenkins (One-at-a-time) de un archivo dado.\n");
        return 0;
    }
    if(strcmp(argv[1], "init") == 0)
    {
        Rep_ repo;
        ugit_init(&repo, argv[2]);
    }
    return 0;
}