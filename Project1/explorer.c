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
    FILE *fp = fopen("seed.txt", "r"); //file pointer, set to read only
    char seedBuffer[MAX_CHAR];         //char array to store seed from seed.text
    int optionsLimit = 5;              //total no. of options
    int randSeed;                      //the seed to be converted to int
    // int randLocationArr[5];            //filled with random values to choose locations
    char locations[6][10] = {"/home", "/proc", "/proc/sys", "/usr", "/usr/bin", "/bin"};

    //check if can't read file
    if (fp != NULL)
    {
        //I'm assuming there will only be one line here
        //for future implementations I'll learn how to only read
        //from the first line if only first line is necessary
        if (fgets(seedBuffer, MAX_CHAR, fp) != NULL)
        {
            randSeed = atoi(seedBuffer);
            fclose(fp);
        }
        else
        {
            printf("Seed file is empty! Exiting...\n");
            exit(1); //EXIT FAIL
        }
    }
    else
    {
        printf("Cannot read from seed file! Exiting...\n");
        exit(1); //EXIT FAIL
    }

    printf("Read seed value: %s\n", seedBuffer);
    printf("\nRead seed value (converted to integer): %d\n", randSeed);

    srand(randSeed);
    // for (int i; i < optionsLimit; i++)
    // {
    //     randLocationArr[i] = rand();
    //     printf("Random num: %d\n", randLocationArr[i]);
    // }

    printf("Let's fork around!\n");

    //Time to fork!
    for (int i = 0; i < optionsLimit; i++)
    {
        // printf("print to check it's working");
        int accessPoint = rand() % 6;
        char cwd[10]; //max string length of wd + null terminator
        // printf("\nLocation ees: %s\n", locations[accessPoint]);
        // char currLocation[10] = &locations[accessPoint];
        if (chdir(locations[accessPoint]) == 0)
        {
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {

                printf("\nSelection #%d: %s [SUCCESS]\n", i + 1, cwd); //using getcwd() to print cwd to terminal
            }
            else
            {
                perror("getcwd() error");
                exit(1);
            }
        }
        else
        {
            printf("\nSelection #%d: Could not enter directory %s [FAILURE]. Exiting...\n", i + 1, locations[accessPoint]);
            exit(1);
        }

        // printf("%d\nLocation is %s\n", accessPoint, locations[accessPoint]);
        // printf("Selection #%d: %s [SUCCESS]\n", i + 1, locations[accessPoint]);
        // char *location[10] = &locations[accessPoint];
        int rc = fork(); //storing fork return value in variable to print to terminal
        int status;      //exit code to pass to parent for printing to terminal
        if (rc < 0)      //fork failure
        {
            fprintf(stderr, "Fork failed! Exiting...\n");
            exit(1);
        }
        else if (rc == 0) //child process
        {
            int PID = (int)getpid(); //casting pidtype to integer for print and store
            printf("    [Child, PID: %d]: Executing 'ls -alh' command...\n", PID);
            char *myArgs[3] = {"ls", "-alh", NULL};
            execvp(myArgs[0], myArgs);
            exit(0); //passing childExitCode to parent, which will access with wait + WEXITSTATUS
        }
        else //parent
        {
            printf("[Parent]: I am waiting for PID %d to finish\n", rc);

            if (waitpid(rc, &status, 0) != -1) //wait for child process to finish, make sure it exits expectedly
            {
                int exitCode = WEXITSTATUS(status);
                printf("[Parent]: Child %d finished with status code %d. Onwards!\n\n", rc, exitCode);
                if (i == optionsLimit - 1) //if process has reached last loop, exit succesfully
                {
                    exit(0);
                }
            }
            else
            {
                printf("Child process failed! Exiting...\n");
                exit(1);
            }
        }
    }
    return 0;
}