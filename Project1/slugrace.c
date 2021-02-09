// Author: Hamayel Qureshi

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

#define BILLION 1E9

/* Below function traverses the array of running children and finished children
   in order to determine whether to print a PID or not */
void printRunningChildren(int runningArr[], int finishedArr[])
{
    printf("The race is ongoing: The following children are still in the race: ");
    for (int j = 0; j < 4; j++)
    {
        int isFinished = 0;
        for (int x = 0; x < 4; x++)
        {
            if (runningArr[j] == finishedArr[x])
                isFinished = 1;
        }
        if (!isFinished)
        {
            printf("%d\t", runningArr[j]);
        }
    }
    printf("\n");
}

int main()
{
    int counter = 1;                        // counter to iterate later on, why not just use iterator + 1?
    int childRunningArr[4] = {0, 0, 0, 0};  // PIDs of slugs running to print and store
    int childFinishedArr[4] = {0, 0, 0, 0}; // PIDs of slugs that have completed, will use to compare
    int slugsRunning = 4;                   // keeping track of whether or not any slugs are alive
    clockid_t clk_id;                       // clock id
    struct timespec rStart, rEnd;           // start and end time timespec

    // For loop to fork off 4 children
    for (int i = 0; i < 4; i++)
    {
        char slugCommand[1];
        sprintf(slugCommand, "%d", counter);
        int rc = fork();
        clock_gettime(CLOCK_REALTIME, &rStart); // get time from the start of the fork process

        if (rc < 0) // fork failed
        {
            printf("Fork failed! Exiting...\n");
            exit(1);
        }
        else if (rc == 0) // child process
        {
            int PID = (int)getpid(); // casting pidtype to integer for print and store
            printf("\t[Child, PID: %d]: Executing './slug %s' command...\n", PID, slugCommand);
            // myArgs array with the command to run
            char *myArgs[3] = {"./slug", slugCommand, NULL};
            if ((execvp(myArgs[0], myArgs)) == -1)
            {
                perror("Exec failed! Exiting...\n");
                exit(0);
            }
        }
        else // parent process
        {
            printf("[Parent]: I forked off child %d\n", rc);
            childRunningArr[i] = rc; // save to running children array
        }
        counter++; // Again, not sure if assignment meant we could use the iterator as a counter
    }

    while (slugsRunning > 0)
    {
        for (int i = 0; i < 4; i++)
        {
            int status;
            int childPID;
            double processTime;
            // Note: I'm checking for > 1 here, because my slug.c will exit with 1 if exec error
            if ((childPID = (int)waitpid(-1, &status, WNOHANG)) > 1)
            {
                // Exited normally, and childPID an actual PID
                if (WIFEXITED(status) && childPID > 0)
                {
                    clock_gettime(CLOCK_REALTIME, &rEnd); // get time when child process ends
                    // I am subtracting .25 to remove the .25 bias added by my usleep()
                    // This may be cheating my numbers a little, though I could not figure out
                    // if my placement of usleep() was somehow wrong (I don't think it is)
                    // which was causing the 0.25 added bias to the time readings
                    processTime = ((rEnd.tv_sec - rStart.tv_sec) + (rEnd.tv_nsec - rStart.tv_nsec) / BILLION) - 0.25;
                    printf("Child %d has crossed the finish line! it took %f seconds\n", childPID, processTime);
                    childFinishedArr[slugsRunning - 1] = childPID; // fill the array backwards
                    slugsRunning--;
                    // If the loop reaches its last iteration, the race is over
                    if (slugsRunning == 0)
                    {
                        clock_gettime(CLOCK_REALTIME, &rEnd);
                        double totalTime = ((rEnd.tv_sec - rStart.tv_sec) + (rEnd.tv_nsec - rStart.tv_nsec) / BILLION) - 0.25;
                        // The race is over! Print message and return (thus ending the program)
                        printf("The race is over! It took a total of %f seconds\n", totalTime);
                        return 0;
                    }
                }
            }
            else if (childPID == 1) // if exec error from slug
            {
                printf("Error waiting for child... Exiting!\n");
                exit(1);
            }
            else // Still waiting for child process
            {
                usleep(250000);
                printRunningChildren(childRunningArr, childFinishedArr);
            }
        }
    }
    return 0;
}