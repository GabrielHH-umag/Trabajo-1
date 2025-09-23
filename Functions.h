#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
void ugit_err(char *msg)
{
    printf("uGitError: %s", msg);
}
void ugit_say(char *msg)
{
    printf("uGit: %s", msg);
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
/*char get_date() // devuelve un string con la fecha actual
{
    char buffer[20];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    //printf("size of tm in bytes: %ld\n", sizeof(struct tm));
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
    return buffer;
}*/
int CreateDir(char *folder_name) // Crea una carpeta
{
	size_t cmd_size = strlen(folder_name) + 20;
	char *cmd = malloc(sizeof(char) * cmd_size);
	snprintf(cmd, cmd_size, "mkdir %s", folder_name);
	if(system(cmd) != 0)
	{
		ugit_err("Couldn't create folder ");
		printf("%s\n", folder_name);
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
        ugit_err("Couldn't change directory to ");
        printf("%s\n", path);
        return 1;
    }
    return 0;
}
int CreateFile(char *file_name) // Crea un archivo
{
	FILE *file = fopen(file_name, "w");
	if (file == NULL)
	{
		ugit_err("Couldn't create file ");
		printf("%s\n", file_name);
		return 1;
	}
	fclose(file);
	return 0;
}
/*int ScanFolder(const char *folderPath)
{
	int fileCount = 0;

	DIR *dir = opendir(folderPath);
	if (!dir) {
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
		ugit_err("Couldn't copy or find file from ");
		printf("%s to %s\n", sourcePath, destPath);
		free(cmd);
		return 1;
	}
	free(cmd);
	return 0;
}