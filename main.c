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
        ugit_say("'commit -m' <your_commit_message> [This function creates a commit with the files in staging area]\n");
        ugit_say("'log' [This function displays info about your actual statement of the repository]\n");
        ugit_say("'checkout' <commit> [This function puts the specified commit on HEAD of the repo]\n");

        return 0;
    }
    if(strcmp(argv[1], "init") == 0)
    {
        Rep_ repo;
        if(ugit_init(&repo, argv[2]) == 1)
            return 1;
    }
    if(strcmp(argv[1], "add") == 0)
    {
        if(argc < 3)
        {
            ugit_err("No file specified to add\nTry: 'add <your_file>' or 'add .' to add all files in your uGit repo\n");
            return 1;
        }
        Rep_ repo;
        if(ugit_add(&repo, argv[2], NULL))
            return 1;
    }
    if(strcmp(argv[1], "commit") == 0 && strcmp(argv[2],"-m") == 0)
    {
        Rep_ repo;
        if(argc < 4)
        {
            ugit_err("No message specified for commit\nTry: 'commit -m <your_message>'\n");
            return 1;
        }
        ugit_commit(&repo, argv[3]);
    }
    if(strcmp(argv[1], "log") == 0)
    {
        Rep_ repo;
        ugit_log(&repo);
    }
    return 0;
}