#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

// include filesystem libraries
#include "filesystem.h"
#include "disk.h"

// TODO : Replace all PRINTFs by writing on terminal (shmem)

// The prompt is an infinite loop. (1)
// stdout is the standard output
// stdin the std input
// stderr the std error

#define MAX_LENGTH 1000
#define SHMEM_KEY 42 // shared memory key

char cmd[MAX_LENGTH]; // command variable

// Shared memory init
char *memory; //memory pointer
int shmemid;  //shared memory id

// PID of the terminal to send signals
pid_t pid;

// By default, positionned on /
int current_node = 0;

// cuts and verify command entered
char *exeCmd(char *command)
{

    // cut at first space to get and verify cmd
    char *ptr = strtok(command, " ");

    if (strcmp(ptr, "help") == 0)
    {
        // TODO: create a function that finds a filename with its inode
        return "---- help command ----\n\n \
            create <filename> ......................... creates a file\n \
            open <filename>  .......................... opens or creates a file\n \
            close <filename> .......................... closes a file\n \
            write <filename> .......................... writes in a file\n \
            mkdir <filename> .......................... creates a directory\n \
            rm <filename> ............................. deletes a file\n \
            rmdir <filename> .......................... deletes an empty directory\n \
            link <filename> <filename> ................ makes a link between a new file and an existing one\n \
            unlink <filename> ......................... deletes a link\n \
            ls <directory> ............................ displays the Inode and the name of the files in the current directory or the specified one\n \
            cd <directory> ............................ change active directory to the specified one\n\n";
    }else if(strcmp(ptr, "mkdir") == 0){
        ptr = strtok(NULL, " ");
        if (ptr == NULL)
        {
            return "ERROR : missing directory name in parameters\n";
        }
        if (fs_mkdir(ptr, current_node))
        {
            perror("Erreur lors de la création du répertoire");
        }
        else
        {
            memset(ptr, 0, strlen(ptr));
            return "\n";
        }
    }else if (strcmp(ptr, "create") == 0)
    {
        ptr = strtok(NULL, " ");
        if (ptr == NULL)
        {
            return "ERROR : missing filename in parameters\n";
        }
        if (fs_create(ptr, current_node))
        {
            perror("Erreur lors de la création du fichier");
        }
        else
        {
            memset(ptr, 0, strlen(ptr));
            return "\n";
        }
    }
    else if (strcmp(ptr, "rm") == 0 || strcmp(ptr, "rmdir") == 0)
    {
        // Get first parameter
        ptr = strtok(NULL, " ");
        if (ptr == NULL)
        {
            return "ERROR : no file specified in parameters\n";
        }else{
            if (fs_rm(ptr, current_node) != 0)
            {
                return "ERROR : no file found\n";
            }
        }
        memset(ptr, 0, strlen(ptr));
        return "\n";
    }
    else if (strcmp(ptr, "ls") == 0)
    {
        char *ret = fs_ls(current_node);
        printf("ls : %s\n", ret);
        return ret;
    }
    else if(strcmp(ptr, "cd") == 0){
        printf("in cd");
        // cut arg
        ptr = strtok(NULL, " ");
        printf("tok : %s\n", ptr);
        if (ptr == NULL){
            return "ERROR : no parameter specified\n";
        }else{
            printf("in else");
            int inode = fs_isDir(current_node, ptr);
            if(inode == -1){
                return "ERROR : not a directory\n";
            }else{
                current_node = inode;
                printf("curr : %d", current_node);
                return "\n";
            }
        }
    }
    else
    {
        return "Commande inconnue.\n";
    }
}

// This function was commented because the commands executed in the child coud not access the filesystem : 
// so we had to juggle with signals to initialise the filesystem in the child and then alternate between the parent and the child :)
/*void getCmd()
{
    size_t size = 1;

    while (size == 1)
    {
        fgets(cmd, MAX_LENGTH, stdin);
        fflush(stdin);
        size = strlen(cmd);
    }

    //get only the command without the \n

    int fd[2];
    pipe(fd);

    pid_t child = fork();

    //char tmp[size - 1];
    //strncpy(tmp, cmd, size - 1);
    //tmp[size] = '\0';

    if (child == -1)
    {
        printf("--- FORK FAILED ---\n");
        exit(-1);
    }
    else if (child != 0)
    { // parent, = shell
        close(fd[1]);

        if (strcmp(cmd, "exit") == 0)
        {
            printf("goodbye\n");
            exit(0);
        }

        printf("commande : %s\n", cmd);

        // wait til the end of the command
        int status = 0;
        waitpid(child, &status, WEXITSTATUS(status));

        // Size of return value
        //int returnSize = 802; //WEXITSTATUS(status);
        //printf("taille = %d\n", returnSize);
        char ret[MAX_LENGTH]; //[returnSize + 1];

        size_t s = read(fd[0], ret, MAX_LENGTH);
        printf("%ld", strlen(ret));
        ret[strlen(ret)] = '\0';

        if (s > 0)
        {
            printf("Retour de la commande %s : %s | %ld\n", cmd, ret, s);

            // send to screen
            strcpy(memory, ret);

            memset(ret, 0, strlen(ret));

            // send signal to screen
            kill(pid, SIGUSR1);
        }
        else
        {
            // Send to screen
            strcpy(memory, "Erreur lors de l'exécution de la commande");

            perror("Erreur lors de l'exécution de la commande ");
            exit(-1);
        }
    }
    else
    {
        // child, = kernel
        close(fd[0]);
        printf("in child \n");

        // execute the command and get the result
        // temp command
        char *tmp = cmd;
        tmp[strcspn(tmp, "\n")] = 0;

        char *res;

        res = exeCmd(tmp);

        printf("res fils : %s\n", res);
        printf("strlen fils : %ld\n", strlen(res));

        // send back to parent
        write(fd[1], res, strlen(res));
        exit(0);
    }
}*/

int main(int argc, char *argv[])
{

    shmemid = shmget((key_t)SHMEM_KEY, 1000, IPC_CREAT | 0750); //create shmem
    memory = shmat(shmemid, NULL, 0);                           //shmem attachement

    if (!argv[1])
    {
        printf("ERREUR : Veuillez renseigner le PID du terminal.\n");
        exit(-1);
    }
    else
    {
        pid = atoi(argv[1]);
    }

    int fd[2];
    pipe(fd);

    pid_t child = fork();

    if (child == 0)
    {
        // init disk
        if (disk_init() < 0)
        {
            perror("Erreur lors de l'initialisation du disque");
        }
        // Init Filesystem
        if (fs_init() < 0)
        {
            perror("Erreur lors de l'initialisation du système de fichiers");
        }

        // child, = kernel
        //close(fd[0]);

        // kernel
        // loops until sig from parent

        kill(getpid(), SIGSTOP); // stops child proc
        while (1)
        {
            // execute the command and get the result
            // temp command
            char *tmp;
            read(fd[0], tmp, MAX_LENGTH);
            tmp[strcspn(tmp, "\n")] = 0;

            char *res = {'\0'};
            res = exeCmd(tmp);

            printf("length : %ld\n", strlen(res));

            // send back to parent
            write(fd[1], res, strlen(res));

            kill(getpid(), SIGSTOP); // stops child proc
        }
    }
    if (child == -1)
    {
        printf("--- FORK FAILED ---\n");
        exit(-1);
    }
    else
    { // parent, = shell
        //close(fd[1]);
        while (1)
        {
            size_t size = 1;

            while (size == 1)
            { // get command
                fgets(cmd, MAX_LENGTH, stdin);
                fflush(stdin);
                size = strlen(cmd);
            }

            if (strcmp(cmd, "exit") == 0)
            {
                printf("goodbye\n");
                exit(0);
            }

            // send command to child
            write(fd[1], cmd, strlen(cmd));
            kill(child, SIGCONT); // resume the child

            // wait til the end of the command
            int status = 0;
            waitpid(child, &status, WUNTRACED);

            // Size of return value
            //int returnSize = 802; //WEXITSTATUS(status);
            //printf("taille = %d\n", returnSize);
            char ret[MAX_LENGTH]; //[returnSize + 1];

            size_t s = read(fd[0], ret, MAX_LENGTH);
            ret[strlen(ret)] = '\0';

            if (s > 0)
            {
                // send to screen
                strcpy(memory, ret);

                memset(ret, 0, strlen(ret));

                // send signal to screen
                kill(pid, SIGUSR1);
            }
            else
            {
                // Send to screen
                strcpy(memory, "Erreur lors de l'exécution de la commande");

                perror("Erreur lors de l'exécution de la commande ");
                exit(-1);
            }
        }
    }

    /*   while (1)
    {
        getCmd();
    }*/
    // Destroy shared memory
    shmctl(shmemid, IPC_RMID, NULL);
    // "Destroy" disk
    disk_deinit();
    exit(0);
}