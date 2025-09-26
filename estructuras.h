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
  char id[41];
  char *msg;
  char *fecha;
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
