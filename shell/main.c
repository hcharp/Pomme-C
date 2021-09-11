#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>


// The prompt is an infinite loop. (1)
// stdout is the standard output
// stdin the std input
// stderr the std error

int main()
{
    char cmd[100], command[100], *parameters[20]; // 2 arrays for the commands + 1 for the parameters' POINTERS

    //environment variable
    char *envp[] = { (char *) "PATH=/bin", 0 }; // the commands will be in /bin

    while (1) // infinite loop
    {
        type_prompt(); // (1)
        read_command (command, parameters); // input

        if (fork() != 0) // if parent + create child = kernel
        {    // wait for command from user
            sscanf(cmd, "/s");

            // send to kernel - which will then send to screen
            // kernel is the child
            // sends to child
            // needs a pipe fd with two ends

            int fd[2]; // pipe
            pipe(fd);
            close (fd[0]); // close reading
            write (fd[1], cmd, 100);
            close (fd[1]); // close writing
        }

        else // if child ; so: if kernel
        {
            // kernel sends to screen
            // printf or signals??
            // screen is NOT a child
            // screen is a simple terminal
            return 0;

        }
    if (strcmp (command, "exit") == 0)
    {
        break;
    }

}