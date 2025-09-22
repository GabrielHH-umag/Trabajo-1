#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "commands.h"
//#include "estructuras.h"
int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        ugit_err("No arguments in\nTry: '[--help] [-h]'\n");
        return 1;
    }
    if(strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
    {
        ugit_say("'init' <your_repo_name> [This command initialize your repo, and creates a directory with .ugit files]\n");
        ugit_say("'add' <add_your_file> [This function adds a file on the stagin area in your repo]\n");
        return 0;
    }
    if(strcmp(argv[1], "init") == 0)
    {
        Rep_ repo;
        if(ugit_init(&repo, argv[2]))
            return 1;
    }
    if(strcmp(argv[1], "add") == 0)
    {
        Rep_ repo;
        if(ugit_add(&repo, NULL, NULL))
            return 1;
    }
    if(strcmp(argv[1],"commit") == 0 && strcmp(argv[2],"-m") == 0)
    {
        ugit_commit("Eiyou");
    }
    return 0;
}