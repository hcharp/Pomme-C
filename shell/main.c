#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

// TODO : Replace all PRINTFs by writing on terminal (shmem)

// The prompt is an infinite loop. (1)
// stdout is the standard output
// stdin the std input
// stderr the std error

#define MAX_LENGTH 100
#define SHMEM_KEY 42 // shared memory key

char cmd[MAX_LENGTH]; // command variable

// Shared memory init
char *memory; //memory pointer
int shmemid;  //shared memory id

// PID of the terminal to send signals
pid_t pid;

// temp
/*
void temp()
{
    char cmd[100], command[100], *parameters[20]; // 2 arrays for the commands + 1 for the parameters' POINTERS

    //environment variable
    char *envp[] = {(char *)"PATH=/bin", 0}; // the commands will be in /bin

    // Pipe init
    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    while (1) // loop
    {

        if (pid != 0 && pid != -1) // we're in the parent proc : shell
        {
            close(fd[0]); // close the reading of the pipe

            // wait for command from user
            sscanf(cmd, "%s");

            // send to kernel - which will then send to screen
            // kernel is the child
            // sends to child
            write(fd[1], cmd, 100);
        }

        else if (pid == 0) // child proc : kernel
        {
            close(fd[1]); // close the writing of the pipe

            // Shell sends cmd to kernel
            // Kernel sends res to Shell then to screen
            // Shell then sends result to screen
            // printf or signals?? We need to use IPC, so shmem
            // screen is NOT a child
            // screen is a simple terminal
            exit(0);
        }
        else
        {
            printf("ECHEC LORS DU FORK\n");
            exit(0);
        }

        if (strcmp(command, "exit") == 0)
        {
            break;
        }
    }
}
*/
void getCmd()
{
    printf("> ");
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
        int returnSize = WEXITSTATUS(status);
        printf("taille = %d\n", returnSize);
        char ret[returnSize+1];

        size_t s = read(fd[0], ret, returnSize);
        ret[returnSize] = '\0';
        
        if (s > 0)
        {
            printf("Retour de la commande %s : %s | %ld\n", cmd, ret, s);

            // send to screen
            strcpy(memory, ret);

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
        char *res = cmd;
        res[strcspn(res, "\n")] = 0;

        printf("res fils : %s\n", res);
        printf("strlen fils : %ld\n", strlen(res));

        // send back to parent
        write(fd[1], res, strlen(res));
        exit(strlen(res));
    }
}

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

    while (1)
    {
        getCmd();
    }

    exit(0);
}