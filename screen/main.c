#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>


// execute a C programm
// wait for data to display

int main()
{
    printf("Pomme-C Terminal");
    printf("Enter a command. Type 'help' to get a list of avalaible commands.");

    char cmd[100];

    while (1)
    {
        printf("$ ");
        fflush(stdin);
        scanf("%s", cmd);

        if (strcmp (cmd, "help") == 0)
        {
            printf("\n");
            printf("create <filename> <mode> .................. creates a file and displays its inode");
            printf("open <filename> <mode> .................... opens a file or create it and displays its inode");
            printf("close <filename> .......................... closes a file"); // TODO: create a function that finds a filename with its inode
            printf("write <filename> .......................... writes in a file");
            printf("mkdir <filename> .......................... creates a directory");
            printf("rmdir <filename> .......................... deletes an empty directory");
            printf("link <filename> <filename> ................ makes a link between a new file and an existing one");
            printf("unlink <filename> ......................... deletes a link");
        }

        else if (strcmp (cmd, "create") == 0)
        {
            // execute the corresponding C programm
        }

        else if (strcmp (cmd, "open") == 0)
        {
            // execute the corresponding C programm
        }

        else if (strcmp (cmd, "close") == 0)
        {
            // execute the corresponding C programm
        }

        else if (strcmp (cmd, "write") == 0)
        {
            // execute the corresponding C programm
        }

        else if (strcmp (cmd, "mkdir") == 0)
        {
            // execute the corresponding C programm
        }

        else if (strcmp (cmd, "rmdir") == 0)
        {
            // execute the corresponding C programm
        }
        else if (strcmp (cmd, "link") == 0)
        {
            // execute the corresponding C programm
        }
        else if (strcmp (cmd, "unlink") == 0)
        {
            // execute the corresponding C programm
        }


    }
}