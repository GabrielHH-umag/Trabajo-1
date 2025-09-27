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
    if (!file)
		return 1;
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
void generate_commit_id(const char *input, char *output)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char *)input, strlen(input), hash);
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
        sprintf(output + (i * 2), "%02x", hash[i]);
    output[SHA_DIGEST_LENGTH * 2] = '\0';
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
	FILE *file = fopen(filename, "rb"); 
	if (!file) 
	{ 
		printf("No se puede abrir el archivo '%s'\n", filename);
		perror("ERROR");
		exit(EXIT_FAILURE);
	} 
	// Determinar el size del archivo 
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
	unsigned int hash = jenkinsHash(buffer, fileSize); 
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
int FileExists(const char *path) // Chequea si existe un archivo
{
	FILE *file = fopen(path, "r");
	if (file)
		return 1;
	return 0;
}
void get_date(char *buffer, size_t size)
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d",
			 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			 tm.tm_hour, tm.tm_min, tm.tm_sec);
}
int CreateDir(char *folder_name) // Crea una carpeta
{
	size_t cmd_size = strlen(folder_name) + 20;
	char *cmd = malloc(sizeof(char) * cmd_size);
	snprintf(cmd, cmd_size, "mkdir \"%s\"", folder_name);
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
	snprintf(cmd, cmd_size, "cp \"%s\" \"%s\"", sourcePath, destPath);
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
        ugit_err("Unable to find repo_data.txt\n");
        return 1;
    }
    char buffer[256];
    if (fscanf(repodata, "name: %[^\n]\n", buffer) == 1)
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
void get_user(char *buffer, size_t size)
{
    char *user = getenv("USER");
    if (user)
		strncpy(buffer, user, size);
    else
		strncpy(buffer, "Unknown", size);
}
int LoadCommitsData(Rep_ *repo)
{
	/*PROBLEMA:
	Cambiar el nombre a las carpetas Commits
	Seria mas eficiente llamar a las carpetas con el nombre "commit_1, commit_2"
	asi sabremos el nombre del directorio y podremos abrirlo para leer la informacion
	*/ //Solucionado(?
	if(DirExists("./.ugit") == 0)
    {
        ugit_err("No .ugit repository found\nTry: 'init <your_repo_name>' or 'init'\n");
        return 1; 
    }
	repo->commits = malloc(sizeof(Commit_) * (repo->num_commit + 1));
	for(int i = 0; i < repo->num_commit; i++)
	{
		char buffer[64];
		snprintf(buffer, sizeof(buffer), "./.ugit/commits/commit_%d/commit_data.txt", i + 1);
		FILE *CommitData = fopen(buffer, "r");
		if(!CommitData)
		{
			ugit_err("Couldn't open commit_data.txt in commit directory: '%s'\n", buffer);
			free(repo->commits);
			return 1;
		}
		fscanf(CommitData, "id: %40s\n", repo->commits[i].id);
		char author_buffer[256];
		fscanf(CommitData, "author: %[^\n]\n", author_buffer);
		repo->commits[i].autor = malloc(strlen(author_buffer) + 1);
		if(repo->commits[i].autor)
			strcpy(repo->commits[i].autor, author_buffer);
		else
		{
			ugit_err("Couldn't assign memory for author buffer\n");
			free(repo->commits);
			fclose(CommitData);
			return 1;
		}
		char date_buffer[64];
		fscanf(CommitData, "date: %[^\n]\n", date_buffer);
		repo->commits[i].fecha = malloc(strlen(date_buffer) + 1);
		if(repo->commits[i].fecha)
			strcpy(repo->commits[i].fecha, date_buffer);
		else
		{
			ugit_err("Couldn't assign memory for date buffer\n");
			free(repo->commits);
			fclose(CommitData);
			return 1;
		}
		char message_buffer[512];
		fscanf(CommitData, "message: %[^\n]\n", message_buffer);
		repo->commits[i].msg = malloc(strlen(message_buffer) + 1);
		if(repo->commits[i].msg)
			strcpy(repo->commits[i].msg, message_buffer);
		else
		{
			ugit_err("Couldn't assign memory for message buffer\n");
			free(repo->commits);
			fclose(CommitData);
			return 1;
		}
		fclose(CommitData);
		/*
		DIR *dir = opendir(buffer);
        struct dirent *entry;
        if (dir)
        {
            //Recorre hasta el ultimo archivo en '.'
            while ((entry = readdir(dir)) != NULL)
			{
				FILE *CommitData = fopen("commit_data.txt", "r");
				fscanf(CommitData, "id: %40s\n", repo->commits[i].id);
				fscanf(CommitData, "author: %s\n", repo->commits[i].autor);
				fscanf(CommitData, "date: %s\n", repo->commits[i].fecha);
				fscanf(CommitData, "message: %s\n", repo->commits[i].msg);
			}
		}*/
	}
	return 0;
}

//Ejemplo
/*
Commits
|________"Commit 1: Wea reculia"
|		  |______commit_data.txt
|		  |______file1.txt
|		  |______file2.txt
|
|________"Commit 2: Wea reculia recontra culia 2"
		  |______commit_data.txt
		  |______file1.txt
		  |______file2.txt
		  |______file3.txt


*/