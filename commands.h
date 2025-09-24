#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include "estructuras.h"
#include "Functions.h"
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
                { 
                    /* solo archivos normales
                     Calcular hash del archivo fuente*/
                    char hash_src[41] = {0};
                    if (hash_file_sha1(entry->d_name, hash_src) != 0) 
                    {
                        ugit_err("Failed to hash file '%s'\n", entry->d_name);
                        continue;
                    }
                    char staging_path[512];
                    snprintf(staging_path, sizeof(staging_path), "./.ugit/staging/%s", entry->d_name);
                    // Verificar si el archivo ya existe en staging
                    int file_exists = 0;
                    FILE *f = fopen(staging_path, "rb");
                    if (f) 
                    {
                        fclose(f);
                        file_exists = 1;
                        char hash_staging[41] = {0};
                        if (hash_file_sha1(staging_path, hash_staging) == 0) {
                            if (strcmp(hash_src, hash_staging) == 0) {
                                ugit_say("File '%s' is unchanged, not added to staging\n", entry->d_name);
                                continue;
                            }
                        }
                    }
                    if (CopyFile(entry->d_name, "./.ugit/staging/") == 0) 
                    {
                        if (!file_exists) {
                            repo->num_stage++;
                        }
                        ugit_say("File '%s' added to the staging area\n", entry->d_name);
                    } 
                    else 
                    {
                        ugit_err("Failed to add file '%s'\n", entry->d_name);
                    }
                }
            }
            closedir(dir);
        }
        Update_RepoData(repo);
        return 0;
    }
    // Caso: agregar un archivo específico
    if (filename != NULL)
    {
        printf("%s\n", filename);
        // Calcular hash del archivo fuente
        char hash_src[41] = {0};
        if (hash_file_sha1(filename, hash_src) != 0)
        {
            ugit_err("Failed to hash file '%s'\n", filename);
            return 1;
        }
        char staging_path[512];
        snprintf(staging_path, sizeof(staging_path), "./.ugit/staging/%s", filename);
        // Verificar si el archivo ya existe en staging
        int file_exists = 0;
        FILE *f = fopen(staging_path, "rb");
        if (f) 
        {
            fclose(f);
            file_exists = 1;
            // Calcular hash del archivo en staging
            char hash_staging[41] = {0};
            if (hash_file_sha1(staging_path, hash_staging) == 0) 
            {
                if (strcmp(hash_src, hash_staging) == 0) 
                {
                    ugit_say("File '%s' is unchanged, not added to staging\n", filename);
                    return 0;
                }
            }
        }
        if (CopyFile(filename, "./.ugit/staging/") == 0) 
        {
            if (!file_exists) 
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
    LoadRepoData(repo);
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
    
    size_t path_len = strlen("./.ugit/commits/") + strlen(message) + 3;
    char *buffer = malloc(path_len);
    if (!buffer)
    {
        ugit_err("Couldn't assign memory for buffer path\n");
        return 1;
    }
    if(repo->num_stage == 0)
    {
        ugit_err("No files in staging area to commit\n");
        free(buffer);
        return 1;
    }
    snprintf(buffer, path_len, "./.ugit/commits/%s", message);
    if(CreateDir(buffer))
    {
        free(buffer);
        return 1;
    }
    
    //Flag: todo bien
    DIR *dir;
    struct dirent *entry;
    dir = opendir("./.ugit/staging");
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