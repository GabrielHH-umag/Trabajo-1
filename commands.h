#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "estructuras.h"
#include "Functions.h"
int ugit_init(Rep_ *repo, char *name)
{
    //Se inicializa el repositorio
    repo->num_historial = 0;
    repo->num_stage = 0;
    if(name != NULL)
    {
        repo->nombre = malloc(sizeof(char) * (strlen(name) + 1));
        if(repo->nombre == NULL)
        {
            ugit_err("No se pudo asignar memoria\n");
            return 1;
        }
        //Nombre aceptado --> Crear carpeta
        if(CreateDir(name) != 0)
            return 1;
        if(ChangeDir(name) != 0)
            return 1;
        if(CreateDir(".ugit") != 0)
            return 1;
        if(CreateDir("./.ugit/commits ./.ugit/staging") != 0)
            return 1;
        ugit_say("Initialized empty uGit repository named: ");
        printf("'%s'\n", repo->nombre);
        //Crear archivo repo_data.txt dentro de .ugit
        /*
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
        */
    }
    else
    {
        CreateDir(".ugit");
        CreateDir("./.ugit/commits ./.ugit/staging");
        ugit_say("Initialized empty uGit repository\n");
    }
    return 0;
}
int ugit_add(Rep_ *repo, char *filename, char *content)
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
    return 0;
}
void ugit_log()
{

}