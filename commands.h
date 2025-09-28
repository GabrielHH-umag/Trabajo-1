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
            ugit_err("Couldn't assign memory for repo name\n");
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
        if(CreateDir("./.ugit/commits"))
            return 1;
        if(CreateDir("./.ugit/staging"))
            return 1;
        //Crear archivo repo_data.txt dentro de .ugit
        if(Update_RepoData(repo))
            return 1;
        ugit_say("Initialized an empty uGit repository named: %s\n", repo->nombre);
        ugit_say("You can now open your repo folder with --> 'cd %s' and get started with your uGit repo\n", repo->nombre);
        return 0;
    }
    else
    {
        if(CreateDir(".ugit"))
            return 1;
        if(CreateDir("./.ugit/commits"))
            return 1;
        if(CreateDir("./.ugit/staging"))
            return 1;
        if(Update_RepoData(repo))
            return 1;
        ugit_say("Initialized an empty uGit repository\n");
    }
    return 0;
}
int ugit_add(Rep_ *repo, char *filename, char *content)
{
    //Carga los datos
    LoadRepoData(repo);
    //Verifica carpetas y las crea si no existen
    if(DirExists("./.ugit/staging") == 0 || DirExists("./.ugit/commits") == 0)
    {
        ugit_say("Warning: Creating missing .ugit subdirectories\n");
        CreateDir("./.ugit/staging");
        CreateDir("./.ugit/commits");
    }

    //add . (Agregar todo a staging)
    if (strcmp(filename, ".") == 0)
    {
        DIR *dir = opendir(".");
        struct dirent *entry;
        if (dir)
        {
            //Recorre hasta el ultimo archivo en '.'
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
                    if (FileExists(staging_path)) 
                    {
                        file_exists = 1;
                        char hash_staging[41] = {0};
                        if (hash_file_sha1(staging_path, hash_staging) == 0)
                        {
                            if (strcmp(hash_src, hash_staging) == 0)
                            {
                                ugit_say("File '%s' is unchanged, not added to staging\n", entry->d_name);
                                continue;
                            }
                        }
                    }
                    //Copia el archivo en entrada a staging
                    if (CopyFile(entry->d_name, "./.ugit/staging/") == 0) 
                    {
                        if (!file_exists)
                            repo->num_stage++;
                        ugit_say("File '%s' added to the staging area\n", entry->d_name);
                    } 
                    else
                    {
                        ugit_err("Failed to add file '%s'\n", entry->d_name);
                        closedir(dir);
                        return 1;
                    }
                }
            }
            closedir(dir);
        }
        Update_RepoData(repo);
        return 0;
    }
    //add <archivo> (Agregar archivo especifico a staging)
    if (filename != NULL)
    {
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
        if (FileExists(staging_path)) 
        {
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
        //Copia el archivo
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
    if(LoadRepoData(repo))
        return 1;

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
    snprintf(buffer, path_len, "./.ugit/commits/commit_%d", repo->num_commit + 1); //<----
    if(CreateDir(buffer))
    {
        free(buffer);
        return 1;
    }
    //Flag: todo bien
    repo->commits = realloc(repo->commits, sizeof(Commit_) * (repo->num_commit + 1));
    DIR *dir = opendir("./.ugit/staging");
    struct dirent *entry;
    repo->commits[repo->num_commit].msg = strdup(message);
    char fecha[64];
    get_date(fecha, sizeof(fecha));
    repo->commits[repo->num_commit].fecha = strdup(fecha);
    char autor[128];
    get_user(autor, sizeof(autor));
    repo->commits[repo->num_commit].autor = strdup(autor);
    Archivo_ *archs = malloc(sizeof(Archivo_) * 100); // Ajusta el tamaño según necesidad
    char tree[1024] = "";
    int count = 0;
    if(dir)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type == DT_REG)
            {
                archs[count].nombre = strdup(entry->d_name);
                strcat(tree, entry->d_name);
                strcat(tree, " ");
                CopyFile(entry->d_name, buffer);
                count++;
            }
        }
        closedir(dir);
        repo->commits[repo->num_commit].archivo = archs;
        repo->commits[repo->num_commit].num_archivos = count;
        char data[2048];
        snprintf(data, sizeof(data), "tree: %s\nauthor: %s\ndate: %s\nmessage: %s\n", tree, repo->commits[repo->num_commit].autor, repo->commits[repo->num_commit].fecha, repo->commits[repo->num_commit].msg);
        char commit_id[41];
        generate_commit_id(data, commit_id);
        strncpy(repo->commits[repo->num_commit].id, commit_id, sizeof(repo->commits[repo->num_commit].id));
        char commit_data[512];
        snprintf(commit_data, sizeof(commit_data), "%s/commit_data.txt", buffer);
        FILE *f = fopen(commit_data, "w");
        if (f) 
        {
            fprintf(f, "id: %s\nauthor: %s\ndate: %s\nmessage: %s\n", repo->commits[repo->num_commit].id, repo->commits[repo->num_commit].autor, repo->commits[repo->num_commit].fecha, repo->commits[repo->num_commit].msg);
            fclose(f);
        }
        else 
            ugit_err("Couldn't create commit_data.txt file\n");
        ugit_say("All files added to commit '%s'\n", message);
        free(archs);
        free(buffer);
        repo->num_commit++;
        Update_RepoData(repo);
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
int ugit_log(Rep_ *repo)
{
    if(LoadRepoData(repo))
        return 1;
    if(LoadCommitsData(repo))
        return 1;
    for(int i = 0; i < repo->num_commit; i++)
    {
        printf("Commit %d\n", i + 1);
        printf("ID: %s\n", repo->commits[i].id);
        printf("Author: %s\n", repo->commits[i].autor);
        printf("Date: %s\n", repo->commits[i].fecha);
        printf("message: %s\n\n", repo->commits[i].msg);
    }
    //GetUser(&repo);
    return 0;
}

int ugit_checkout(Rep_ *repo, char *id)
{
    if (LoadRepoData(repo))
        return 1;
    if (LoadCommitsData(repo))
        return 1;
    int found = -1;
    int prefix_len = strlen(id);
    for (int i = 0; i < repo->num_commit; i++) 
    {
        if (strncmp(repo->commits[i].id, id, prefix_len) == 0) 
        {
            if (found != -1) 
            {
                ugit_err("Hash prefix is not unique\n");
                return 1;
            }
            found = i;
        }
    }
    if (found == -1) 
    {
        ugit_err("Hash prefix doesn't correspond to any commit\n");
        return 1;
    }
    char commit_dir[256];
    snprintf(commit_dir, sizeof(commit_dir), "./.ugit/commits/commit_%d", found + 1);
    DIR *dir = opendir(commit_dir);
    if (!dir) 
    {
        ugit_err("Failed to open commit directory\n");
        return 1;
    }
    struct dirent *entry;
    int restored = 0;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (entry->d_type != DT_REG || strcmp(entry->d_name, "commit_data.txt") == 0)
            continue;
        char src[512], dst[512];
        snprintf(src, sizeof(src), "%s/%s", commit_dir, entry->d_name);
        snprintf(dst, sizeof(dst), "./%s", entry->d_name);
        // Restaurar solo si el archivo es distinto
        char hash_src[41] = {0};
        char hash_dst[41] = {0};
        int src_ok = hash_file_sha1(src, hash_src);
        int dst_ok = hash_file_sha1(dst, hash_dst);
        if (src_ok != 0 || (dst_ok == 0 && strcmp(hash_src, hash_dst) == 0)) 
        {
            continue;
        }
        if (CopyFile(src, dst) == 0) 
        {
            ugit_say("File restored %s\n", entry->d_name);
            restored++;
        } 
        else 
        {
            ugit_err("Failed to restore file: %s\n", entry->d_name);
        }
    }
    closedir(dir);
    if (restored == 0) 
    {
        ugit_say("Couldn't restore any files/Files where unchanged\n");
    } 
    else 
    {
    ugit_say("Switched to Commit %d\nID: %s\nFiles restored: %d\n", found + 1, repo->commits[found].id, restored);
    }
    return 0;
}