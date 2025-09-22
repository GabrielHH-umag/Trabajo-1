#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "estructuras.h"
#include "Functions.h"
void ugit_err(char* msg)
{
    printf("ugitError: %s", msg);
}
int ugit_init(Rep_* repo, char* name)
{
    //Se inicializa el repositorio
    char *command = NULL;
    repo->num_historial = 0;
    repo->num_stage = 0;
    if(name != NULL)
    {
        /*if(name == NULL || strlen(name) == 0)
        {
            ugit_err("No name for the repository\nTry: 'init <your_repo_name>'\n");
            return 1;
        }*/
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
            ugit_err("Couldn't assign memory for .ugit 'create command'\n");
            return 1;
        }
        strncpy(command, "mkdir ", cmd_size);
        strncat(command, repo->nombre, cmd_size - strlen(command) - 1); // command = mkdir reponame
        if(system(command) != 0)
        {
            printf("ugitError: Couldn't create repo folder named: %s\n", repo->nombre);
            free(command);
            return 1;
        }

        //Crear carpeta .ugit dentro de la carpeta principal
        int dotugit_cmd_size = strlen(repo->nombre) + 30;
        char *dotugit_command = malloc(sizeof(char) * dotugit_cmd_size);
        if(dotugit_command == NULL)
        {
            ugit_err("Couldn't assign memory for .ugit create command\n");
            free(command);
            return 1;
        }
        snprintf(dotugit_command, dotugit_cmd_size, "mkdir %s/.ugit", repo->nombre);
        if(system(dotugit_command) != 0)
        {
            printf("ugitError: Couldn't create .ugit folder in repo %s\n", repo->nombre);
            free(command);
            free(dotugit_command);
            return 1;
        }
        free(dotugit_command);
        //Cambiar directorio a la carpeta del repo
        dotugit_command = malloc(sizeof(char) * dotugit_cmd_size);
        if(dotugit_command == NULL)
        {
            ugit_err("Couldn't assign memory for .ugit create command\n");
            free(command);
            return 1;
        }
        snprintf(dotugit_command, dotugit_cmd_size, "cd %s", repo->nombre);
        if(system(dotugit_command) != 0)
        {
            ugit_err("Couldn't change directory to repo folder\n");
            free(command);
            free(dotugit_command);
            return 1;
        }
        free(dotugit_command);

        //Crear archivo repo_data.txt dentro de .ugit
        char txt_path[256];
        snprintf(txt_path, sizeof(txt_path), "%s/.ugit/repo_data.txt", repo->nombre);
        FILE *txt_file = fopen(txt_path, "w");
        if(txt_file == NULL)
        {
            ugit_err("Couldn't create repo_data.txt file\n");
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
    }
    else
    {
        if(system("mkdir .ugit") != 0)
            ugit_err("Unable to create .ugit folder in repo\n");
        else
            printf("Initialized empty uGit repo\n");
    }
    free(command);
    return 0;
}
int ugit_add(Rep_ *repo, char* filename, char* content)
{
    if(Directory_exists(".ugit") == 0)
    {
        ugit_err("No .ugit repository found\nTry: 'init <your_repo_name>'\n");
        return 1;
    }
    printf(".ugit Already exists\n");
    return 0;
}
int ugit_commit(const char* message)
{

}
void ugit_log()
{

}