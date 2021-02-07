#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
//setting MAX_CHAR for seedBuffer here to avoid running sizeof
//function unnecessarily
#define MAX_CHAR 255

/*Takes a seed, minimum value, and maximum value, returns random value */
int getRandom(int seed, int min, int max)
{
    srand(seed);                           //seed the randomizer
    return rand() % (max + 1 - min) + min; //return pseudorandom value
}

int main(int argc, char *argv[])
{
    int counter = 1; //counter to iterate later on, (question for myself: why can't we just use iterator variable?)
    int childPidArr[4];

    //Time to fork!
    for (int i = 0; i < 4; i++)
    // while (1)
    {
        // usleep(250000);
        // fflush(NULL);
        int rc = fork(); //storing fork return value in variable to print to terminal
        int status;      //exit code to pass to parent for printing to terminal
        char slugCommand[2];
        sprintf(slugCommand, "%d", counter); //store counter as string to char slugCommand for argument
        if (rc < 0)                          //fork failure
        {
            fprintf(stderr, "Fork failed! Exiting...\n");
            exit(1);
        }
        else if (rc == 0) //child process
        {
            int PID = (int)getpid(); //casting pidtype to integer for print and store
            printf("    [Child, PID: %d]: Executing './slug %s' command...\n", PID, slugCommand);
            char *myArgs[3] = {"./slug", slugCommand, NULL};
            execvp(myArgs[0], myArgs);

            // exit(counter); //exiting with the counter value as an exit code to pass to parent
        }
        else //parent
        {

            printf("[Parent]: I forked off child %d\n", rc);
            // wait(NULL);
            // waitpid(rc, &status, 0);
            // exit(0);

            if (waitpid(-1, &status, WNOHANG) != -1) //WNOHANG prevents blocking
            {
                int exitCode = WEXITSTATUS(status);
                // printf("[Parent]: Child %d finished with status code %d. Onwards!\n", rc, exitCode);
                // if (counter == 4) //if process has reached last loop, exit succesfully
                // {
                //     exit(0);
                // }
            }
            else
            {
                printf("Child process failed! Exiting...\n");
                exit(1);
            }
        }
        counter++;
    }

    return 0;
}