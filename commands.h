#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include "estructuras.h"
#include "Functions.h"
int Update_RepoData(Rep_ *repo)
{
	FILE *txt_file = fopen(".ugit/repo_data.txt", "a+");
	if(txt_file == NULL)
	{
		ugit_err("Couldn't create file .ugit/repo_data.txt\n");
		return 1;
	}
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
        if(CreateDir(name) != 0)
            return 1;
        if(ChangeDir(name) != 0)
            return 1;
        if(CreateDir(".ugit") != 0)
            return 1;
        if(CreateDir("./.ugit/commits ./.ugit/staging") != 0)
            return 1;
        //Crear archivo repo_data.txt dentro de .ugit
        if(Update_RepoData(repo))
            return 1;
        ugit_say("Initialized an empty uGit repository named: ");
        printf("%s\n", repo->nombre);
        printf("You can now open your repo folder with --> 'cd %s'\nand get started with your uGit repo\n", repo->nombre);
        return 0;
    }
    else
    {
        CreateDir(".ugit");
        CreateDir("./.ugit/commits ./.ugit/staging");
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
        ugit_say("Adding all files in the current directory to the staging area\n");

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
        ugit_say("Adding file to the staging area: ");
        printf("%s\n", filename);

        if (CopyFile(filename, "./.ugit/staging/") == 0)
        {
            repo->num_stage++;
            Update_RepoData(repo);
            ugit_say("File added to the staging area: ");
            printf("%s\n", filename);
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
int ugit_commit(const char* message)
{
    return 0;
}
void ugit_log()
{

}