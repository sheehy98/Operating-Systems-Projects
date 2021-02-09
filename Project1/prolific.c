#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
// setting MAX_CHAR for seedBuffer here to avoid running sizeof
// function unnecessarily
#define MAX_CHAR 255

/* Takes a seed, minimum value, and maximum value, returns random value */
int getRandom(int seed, int min, int max)
{
    srand(seed);                           //seed the randomizer
    return rand() % (max + 1 - min) + min; //return pseudorandom value
}

int main(int argc, char *argv[])
{
    FILE *fp = fopen("seed.txt", "r"); // file pointer, set to read only
    char seedBuffer[MAX_CHAR];         // char array to store seed from seed.text
    int randSeed;                      // the seed to be converted to int
    int numChilds;                     // number of children randomly generated
    int counter = 0;                   // counter to iterate later on, (question for myself: why can't we just use iterator variable?)

    // check if can't read file
    if (fp != NULL)
    {
        // I'm assuming there will only be one line here
        // for future implementations I'll learn how to only read
        // from the first line if only first line is necessary
        if (fgets(seedBuffer, MAX_CHAR, fp) != NULL)
        {
            randSeed = atoi(seedBuffer);
            fclose(fp);
        }
        else
        {
            printf("Seed file is empty! Exiting...\n");
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

    numChilds = getRandom(randSeed, 10, 15); // get the randomly generated number of children
    printf("Random Child Count: %d\n", numChilds);
    printf("Let's fork around!\n");

    int childRandArr[numChilds]; // array for random number for each child

    // filling array with random value for child
    for (int i = 0; i < numChilds; i++)
    {
        childRandArr[i] = rand();
    }

    // Time to fork!
    for (int i = 0; i < numChilds; i++)
    {
        int rc = fork(); // storing fork return value in variable to print to terminal
        int status;      // exit code to pass to parent for printing to terminal
        if (rc < 0)      // fork failure
        {
            fprintf(stderr, "Fork failed! Exiting...\n");
            exit(1);
        }
        else if (rc == 0) // child process
        {
            int PID = (int)getpid(); // casting pidtype to integer for print and store
            int childRandValue = childRandArr[counter];
            int childDelay = (childRandValue % 3) + 1;
            int childExitCode = (childRandValue % 50) + 1;
            printf("    [Child, PID: %d]: I am the child and I will wait %d seconds and exit with code %d\n", PID, childDelay, childExitCode);
            sleep(childDelay);
            printf("    [Child, PID: %d]: Now exiting...\n", PID);
            exit(childExitCode); // passing childExitCode to parent, which will access with wait + WEXITSTATUS
        }
        else // parent
        {
            printf("\n[Parent]: I am waiting for PID %d to finish\n", rc);

            if (waitpid(rc, &status, 0) != -1) // wait for child process to finish, make sure it exits expectedly
            {
                if (WIFEXITED(status))
                {
                    int exitCode = WEXITSTATUS(status);
                    printf("[Parent]: Child %d finished with status code %d. Onwards!\n", rc, exitCode);
                    if (counter == numChilds - 1) // if process has reached last loop, exit succesfully
                    {
                        exit(0);
                    }
                }
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