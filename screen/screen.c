#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#define KEY 42 // shared memory key

// action on signal received ( = writing on shmem)
void action_signal(int num)
{

    if (num == SIGUSR1)
    {
        //SHARED MEMORY
        char *memory; //memory pointer
        int shmemid;  // shmem id

        shmemid = shmget((key_t)KEY, 0, 0); //shmem create
        memory = shmat(shmemid, NULL, 0);   //shmem attach

        printf("%s\n", memory);
        shmdt(memory);
    }
}

int main(int argc, char const *argv[])
{
    //SIGNALS
    struct sigaction action;
    action.sa_handler = action_signal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    printf("Pomme-C Terminal\nPID : %d\n", getpid());
    printf("Enter a command. Type 'help' to get a list of avalaible commands.\n");

    while (1)
    {
        if (sigaction(SIGUSR1, &action, NULL) == -1)
        {
            printf("Impossible d'appeler sigaction\n");
            exit(EXIT_FAILURE);
        }
    }
}