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

// cuts and verify command entered
char* exeCmd(char *command)
{

    // cut at first space to get and verify cmd
    char *ptr = strtok(command, " ");

    if(strcmp(ptr, "help")==0){
        // TODO: create a function that finds a filename with its inode
        return "---- help command ----\n\n \
            create <filename> ......................... creates a file\n \
            open <filename>  .......................... opens or creates a file\n \
            close <filename> .......................... closes a file\n \
            write <filename> .......................... writes in a file\n \
            mkdir <filename> .......................... creates a directory\n \
            rmdir <filename> .......................... deletes an empty directory\n \
            link <filename> <filename> ................ makes a link between a new file and an existing one\n \
            unlink <filename> ......................... deletes a link\n \
            ls <directory> ............................ displays the directory contents\n\n";
    }else{
        return "Commande inconnue.\n";
    }
}

void getCmd()
{

    fgets(cmd, MAX_LENGTH, stdin);
    fflush(stdin);
    size_t size = strlen(cmd);
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
        printf("%ld",strlen(ret));
        ret[strlen(ret)] = '\0';

        if (s > 0)
        {
            printf("Retour de la commande %s : %s | %ld\n", cmd, ret, s);

            // send to screen
            strcpy(memory, ret);

            memset(ret,0,strlen(ret));

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
    { // child, = kernel
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
}

int main(int argc, char *argv[])
{
    // init disk
    if(disk_init() < 0){
        perror("allo");
    }
    // Init Filesystem
    if(fs_init() < 0){
        perror("allo2");
    }

    if(fs_create("test", 0)){
        perror("Erreur lors de la création du fichier");
    }

    if(fs_mkdir("dir_test", 0)){
        perror("Erreur lors de la création du dossier");
    }

    fs_search(0); // ls equivalent

    fs_rm("test", 0);
    fs_search(0); // ls equivalent
    fs_rm("dir_test", 0);
    fs_search(0); // ls equivalent

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

    while (1)
    {
        getCmd();
    }

    disk_deinit();
    exit(0);
}