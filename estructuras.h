#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H
typedef struct
{
  char *nombre;
  char *content;
} Archivo_;

typedef struct
{
  char id[10];
  char msg[200];
  char fecha[50];
  char *autor;
  Archivo_ *archivo;
  int num_archivos;
} Commit_;

typedef struct
{
  char *nombre;
  int num_stage;
  Commit_ *commits;
  int num_commit;
} Rep_;
#endif
