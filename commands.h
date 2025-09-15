#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "estructuras.h"
void ugit_err(char* msg)
{
    printf("ugitErr: %s", msg);
}
int ugit_init(Rep_* repo, char* name)
{
    //Se inicializa el repositorio
    char *command = NULL;
    if(name == NULL || strlen(name) == 0)
    {
        ugit_err("Nombre inexistente o invalido\n");
        return 1;
    }
    repo->num_historial = 0;
    repo->num_stage = 0;
    repo->nombre = malloc(sizeof(char) * (strlen(name) + 1));
    if(repo->nombre == NULL)
    {
        ugit_err("No se pudo asignar memoria\n");
        return 1;
    }
    //Nombre aceptado --> Crear carpeta
    strcpy(repo->nombre, name);
    int cmd_size = strlen(repo->nombre) + 20;
    command = malloc(sizeof(char) * cmd_size);
    if(command == NULL)
    {
        ugit_err("No se pudo asignar memoria\n");
        return 1;
    }
    strncpy(command, "mkdir ", cmd_size);
    strncat(command, repo->nombre, cmd_size - strlen(command) - 1);
    system(command);

    //Crear carpeta .ugit dentro de la carpeta principal
    int dotugit_cmd_size = strlen(repo->nombre) + 30;
    char *dotugit_command = malloc(sizeof(char) * dotugit_cmd_size);
    if(dotugit_command == NULL)
    {
        ugit_err("No se pudo asignar memoria para .ugit\n");
        free(command);
        return 1;
    }
    snprintf(dotugit_command, dotugit_cmd_size, "mkdir %s/.ugit", repo->nombre);
    system(dotugit_command);

    char txt_path[256];
    snprintf(txt_path, sizeof(txt_path), "%s/.ugit/repo_data.txt", repo->nombre);
    FILE *txt_file = fopen(txt_path, "w");
    if(txt_file == NULL)
    {
        ugit_err("No se pudo crear el archivo repo_data.txt\n");
        free(command);
        free(dotugit_command);
        return 1;
    }
    fprintf(txt_file, "num_stage: %d\n", repo->num_stage);
    fprintf(txt_file, "num_commit: %d\n", repo->num_historial);
    for(int i = 0; i < repo->num_historial; i++)
    {
        Commit_ *c = &repo->historial[i];
        fprintf(txt_file, "Commit %d:\n  id: %s\n  msg: %s\n  fecha: %s\n  num_archivos: %d\n", i+1, c->id, c->msg, c->fecha, c->num_archivos);
    }
    fclose(txt_file);

    printf("Initialized empty uGit repository named: '%s'\n", repo->nombre);
    free(command);
    free(dotugit_command);
    return 0;
}
void ugit_add(Rep_ *repo, char* filename, char* content)
{

}
void ugit_commit(const char* message)
{

}
void ugit_log()
{

}