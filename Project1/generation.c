// Author: Hamayel Qureshi

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CHAR 255

/* Takes a seed, minimum value, and maximum value, returns random value */
int getRandom(int seed, int min, int max)
{
    srand(seed);                           // seed the randomizer
    return rand() % (max + 1 - min) + min; // return pseudorandom value
}

/* This function handles all forking and child processes. Refactored into separate function to make it easier
   to call itself recusrively */
int descend(int descendantCount)
{
    // exit condition for recursive calls
    // also what the problem statement asks for!
    // if the child process decrements the count to zero or -ve
    if (descendantCount <= 0)
    {
        exit(0);
    }

    int rc = fork(); // storing fork return value in variable to print to terminal
    int status;      // exit code to pass to parent for printing to terminal

    if (rc < 0) // fork failure
    {
        fprintf(stderr, "Fork failed! Exiting...\n");
        exit(1);
    }
    else if (rc == 0) // child process
    {

        int newDescendantCount = descendantCount - 1;

        printf("\t[Child, PID: %d]: I was called with descendant count=%d. I'll have %d descendants.\n",
               (int)getpid(), descendantCount, newDescendantCount);
        descend(newDescendantCount); // recusively call itself with the new decreased generation count
        exit(newDescendantCount);    // exit with newDescendantCount to pass to parent, should use pipes in the future
    }

    else //parent process
    {
        int pid = (int)getpid();
        printf("[Parent, PID: %d]: I am waiting for PID %d to finish\n", pid, rc);

        if (waitpid(rc, &status, 0) != -1) //wait for child process to finish, make sure it exits expectedly
        {
            descendantCount = WEXITSTATUS(status); /* update descendant count value, I recognize this is a dirty way
                                                     I should probably be using pipes */
            printf("[Parent, PID: %d]: Child %d finished with status code %d. Byeeee :^)\n", pid, rc, descendantCount);
        }
        else
        {
            printf("Child process failed! Exiting...\n");
            exit(1);
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    FILE *fp = fopen("seed.txt", "r"); // file pointer, set to read only
    char seedBuffer[MAX_CHAR];         // char array to store seed from seed.text
    int randSeed;                      // the seed to be converted to int
    int descendantCount;               // number of children randomly generated

    // check if can't read file
    if (fp != NULL)
    {
        if (fgets(seedBuffer, MAX_CHAR, fp) != NULL)
        {
            randSeed = atoi(seedBuffer);
            fclose(fp);
        }
        else
        {
            printf("Seed file is empty! Exiting...\n");
            fclose(fp);
            exit(1); // EXIT FAIL
        }
    }
    else
    {
        printf("Cannot read from seed file! Exiting...\n");
        exit(1); // EXIT FAIL
    }

    printf("Read seed value: %s\n", seedBuffer);
    printf("\nRead seed value (converted to integer): %d\n", randSeed);

    descendantCount = getRandom(randSeed, 7, 10); // parent picks a random number for lifespan count
    printf("Random Descendant Count: %d\n", descendantCount);
    printf("Family reunion time :)\n\n");

    // run recusive fork calls for parent + children
    descend(descendantCount);

    return 0;
}