#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int rc = fork();

    if (rc < 0)
    {
        fprintf(stderr, "Fork failed! Exiting...\n");
        exit(1);
    }
    else if (rc == 0)
    {
        printf("    [Child]: I am the child process with PID %d\n", (int)getpid());
        printf("    [Child]: I will be running the '/bin/ls' command in my Project1 directory with execv\n");
        sleep(1);
        execl("/bin/ls", "ls", "../Project1", NULL);
    }
    else
    {
        printf("[Parent]: I am forking off child with PID %d\n", rc);
        wait(NULL);
    }

    return 0;
}