#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <openssl/sha.h>
#include <stdarg.h>
void ugit_err(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	printf("uGitError: ");
	vprintf(format, args);
	va_end(args);
}
void ugit_say(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	printf("uGit: ");
	vprintf(format, args);
	va_end(args);
}
int hash_file_sha1(const char *filename, char *output)
{
    FILE *file = fopen(filename, "rb");
    if (!file) return 1;
    SHA_CTX ctx;
    unsigned char hash[SHA_DIGEST_LENGTH];
    unsigned char buffer[4096];
    size_t bytes;
    SHA1_Init(&ctx);
    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) != 0)
        SHA1_Update(&ctx, buffer, bytes);
    SHA1_Final(hash, &ctx);
    fclose(file);
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
        sprintf(output + (i * 2), "%02x", hash[i]);
    output[SHA_DIGEST_LENGTH * 2] = '\0';
    return 0;
}
unsigned int jenkinsHash(unsigned char *key, size_t len)
{
	unsigned int hash = 0; 
	for (size_t i = 0; i < len; i++) 
	{ 
		hash += key[i]; 
		hash += (hash << 10); 
		hash ^= (hash >> 6); 
	} 
	hash += (hash << 3); 
	hash ^= (hash >> 11); 
	hash += (hash << 15); 
	return hash; 
} 
// Funcion para leer el contenido de un archivo y calcular el hash
unsigned int hashFile(char *filename) 
{ 
	FILE *file = fopen(filename, "rb"); // Abrir el archivo en modo binario 
	if (!file) 
	{ 
		printf("No se puede abrir el archivo '%s'\n", filename);
		perror("ERROR");
		exit(EXIT_FAILURE);
	} 
	// Determinar el tamanho del archivo 
	fseek(file, 0, SEEK_END); 
	long fileSize = ftell(file); 
	fseek(file, 0, SEEK_SET); 
	// Leer el contenido del archivo 
	unsigned char *buffer = (unsigned char *)malloc(fileSize * sizeof(unsigned char)); 
	if (!buffer) 
	{ 
		perror("No se puede asignar memoria"); 
		fclose(file); 
		exit(EXIT_FAILURE); 
	} 
	fread(buffer, sizeof(unsigned char), fileSize, file); 
	fclose(file); 
	// Calcular el hash del contenido 
	unsigned int hash = jenkinsHash(buffer, fileSize); 
	// Liberar la memoria del buffer 
	free(buffer); 
	return hash; 
}
int DirExists(const char *path) // Chequea si existe un directorio
{
    struct stat stats;
    stat(path, &stats);
    if (S_ISDIR(stats.st_mode))
        return 1;
    return 0;
}
void get_date(char *buffer)
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	snprintf(buffer, 64, "%04d-%02d-%02d %02d:%02d:%02d",
			 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			 tm.tm_hour, tm.tm_min, tm.tm_sec);
}
int CreateDir(char *folder_name) // Crea una carpeta
{
	size_t cmd_size = strlen(folder_name) + 20;
	char *cmd = malloc(sizeof(char) * cmd_size);
	snprintf(cmd, cmd_size, "mkdir %s", folder_name);
	if(system(cmd) != 0)
	{
		ugit_err("Couldn't create folder '%s'\n", folder_name);
		free(cmd);
		return 1;
	}
	free(cmd);
	return 0;
}
int ChangeDir(char *path)
{
    if (chdir(path) != 0)
	{
        ugit_err("Couldn't change directory to '%s'\n", path);
        return 1;
    }
    return 0;
}
int CreateFile(char *file_name) // Crea un archivo
{
	FILE *file = fopen(file_name, "w");
	if (file == NULL)
	{
		ugit_err("Couldn't create file '%s'\n", file_name);
		return 1;
	}
	fclose(file);
	return 0;
}
/*int ScanFolder(const char *folderPath)
{
	int fileCount = 0;

	DIR *dir = opendir(folderPath);
	if (!dir) 
	{
		perror("No se pudo abrir la carpeta");
		return fileCount;
	}

	struct dirent *entry;

	while ((entry = readdir(dir)) != NULL)
	{
		#ifdef _DIRENT_HAVE_D_TYPE
				if (entry->d_type == DT_REG) // Solo archivos normales
		#else
				struct stat path_stat;
				stat(entry->d_name, &path_stat);
				if (S_ISREG(path_stat.st_mode)) // Solo archivos normales
		#endif
		{
			fileCount++;
		}
	}

	closedir(dir);
	return fileCount;
}*/
int CopyFile(const char *sourcePath, const char *destPath) // Copia un archivo
{
	size_t cmd_size = strlen(sourcePath) + strlen(destPath) + 20;
	char *cmd = malloc(sizeof(char) * cmd_size);
	snprintf(cmd, cmd_size, "cp %s %s", sourcePath, destPath);
	if(system(cmd) != 0)
	{
		ugit_err("Couldn't copy or find file from '%s' to '%s'\n", sourcePath, destPath);
		free(cmd);
		return 1;
	}
	free(cmd);
	return 0;
}
int LoadRepoData(Rep_ *repo)
{
    if(DirExists("./.ugit") == 0)
    {
        ugit_err("No .ugit repository found\nTry: 'init <your_repo_name>' or 'init'\n");
        return 1; 
    }
    FILE *repodata = fopen("./.ugit/repo_data.txt", "r");
    if(!repodata)
    {
        ugit_err("Unable to find repo_data.txt");
        return 1;
    }
    char buffer[256];
    if (fscanf(repodata, "name: %255s\n", buffer) == 1)
    {
        repo->nombre = malloc(strlen(buffer) + 1);
        if (repo->nombre)
            strcpy(repo->nombre, buffer);
        else
        {
            ugit_err("Couldn't assign memory for name bufffer\n");
            fclose(repodata);
            return 1;
        }
    }
    else
    {
        ugit_err("Couldn't read repo name\n");
        fclose(repodata);
        return 1;
    }
    fscanf(repodata, "num_stage: %d\n", &repo->num_stage);
	fscanf(repodata, "num_commit: %d\n", &repo->num_commit);
    fclose(repodata);
    return 0;
}
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