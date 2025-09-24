#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include "estructuras.h"
#include "Functions.h"
int Update_RepoData(Rep_ *repo)
{
	FILE *txt_file = fopen(".ugit/repo_data.txt", "w");
	if(txt_file == NULL)
	{
		ugit_err("Couldn't create file .ugit/repo_data.txt\n");
		return 1;
	}
    fprintf(txt_file, "name: %s\n", repo->nombre);
	fprintf(txt_file, "num_stage: %d\n", repo->num_stage);
	fprintf(txt_file, "num_commit: %d\n", repo->num_commit);
	fclose(txt_file);
    return 0;
}
int ugit_init(Rep_ *repo, char *name)
{
    repo->num_commit = 0;
    repo->num_stage = 0;
    if(name != NULL)
    {
        repo->nombre = malloc(sizeof(char) * (strlen(name) + 1));
        if(repo->nombre == NULL)
        {
            ugit_err("No se pudo asignar memoria\n");
            return 1;
        }
        strcpy(repo->nombre, name);
        //Nombre aceptado --> Crear carpeta
        if(CreateDir(name))
            return 1;
        if(ChangeDir(name))
            return 1;
        if(CreateDir(".ugit"))
            return 1;
        if(CreateDir("./.ugit/commits ./.ugit/staging"))
            return 1;
        //Crear archivo repo_data.txt dentro de .ugit
        if(Update_RepoData(repo))
            return 1;
        ugit_say("Initialized an empty uGit repository named: %s", repo->nombre);
        ugit_say("You can now open your repo folder with --> 'cd %s'\nand get started with your uGit repo\n", repo->nombre);
        return 0;
    }
    else
    {
        if(CreateDir(".ugit"))
            return 1;
        if(CreateDir("./.ugit/commits ./.ugit/staging"))
            return 1;
        if(Update_RepoData(repo))
            return 1;
        ugit_say("Initialized an empty uGit repository\n");
    }
    return 0;
}
int ugit_add(Rep_ *repo, char *filename, char *content)
{
    if(DirExists("./.ugit") == 0)
    {
        ugit_err("No .ugit repository found\nTry: 'init <your_repo_name>' or 'init'\n");
        return 1;
    }

    // Asegurar subdirectorios
    if(DirExists("./.ugit/staging") == 0 || DirExists("./.ugit/commits") == 0)
    {
        ugit_say("Warning: Creating missing .ugit subdirectories\n");
        CreateDir("./.ugit/staging");
        CreateDir("./.ugit/commits");
    }

    // Caso: agregar todos los archivos
    if (strcmp(filename, ".") == 0)
    {
        DIR *dir;
        struct dirent *entry;
        dir = opendir(".");
        if (dir)
        {
            while ((entry = readdir(dir)) != NULL)
            {
                if (entry->d_type == DT_REG)
                { // solo archivos normales
                    if (CopyFile(entry->d_name, "./.ugit/staging/") == 0)
                        repo->num_stage++;
                }
            }
            closedir(dir);
        }

        Update_RepoData(repo);
        ugit_say("All files added to the staging area\n");
        return 0;
    }

    // Caso: agregar un archivo específico
    if (filename != NULL)
    {
        printf("%s\n", filename);

        if (CopyFile(filename, "./.ugit/staging/") == 0)
        {
            repo->num_stage++;
            Update_RepoData(repo);
            ugit_say("File '%s' added to the staging area\n", filename);
            return 0;
        } 
        else
        {
            ugit_err("Failed to add file\n");
            return 1;
        }
    }
    return 0;
}
int ugit_commit(Rep_ *repo, const char* message)
{
    if(DirExists("./.ugit") == 0)
    {
        ugit_err("No .ugit repository found\nTry: 'init <your_repo_name>' or 'init'\n");
        return 1;
    }

    // Asegurar subdirectorios
    if(DirExists("./.ugit/staging") == 0 || DirExists("./.ugit/commits") == 0)
    {
        ugit_err("Missing .ugit subdirectories, creating them now\n");
        CreateDir("./.ugit/staging");
        CreateDir("./.ugit/commits");
        return 1;
    }

    char *buffer = malloc(sizeof(char) * strlen(message) + 21);
    snprintf(repo->nombre, sizeof(repo->nombre), "./ugit/commits/'%s'", message);
    if(CreateDir(buffer))
    {
        free(buffer);
        return 1;
    }
    if(ChangeDir("./.ugit/staging"))
    {
        free(buffer);
        return 1;
    }
    DIR *dir;
    struct dirent *entry;
    dir = opendir(".");
    if(dir)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type == DT_REG)
            { // solo archivos normales
                if (CopyFile(entry->d_name, buffer) == 0)
                    repo->num_commit++;
            }
        }
        closedir(dir);
        Update_RepoData(repo);
        ugit_say("All files added to commit '%s'\n", message);
        free(buffer);
        return 0;
    }
    else
    {
        ugit_err("Failed to open current directory\n");
        free(buffer);
        return 1;
    }
    return 0;
}
void ugit_log()
{

}