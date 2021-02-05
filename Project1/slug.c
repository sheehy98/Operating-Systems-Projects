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

int getSeed(int arg)
{
    //originally, I wanted to do the seed_slug_x.txt more elegantly with
    //the integer being passed from command line argument being concatenated
    //in place of the x. however, hard coding the array and accessing it via
    //index will suffice for now as I relearn C
    int pid = (int)getpid();   //process id
    char seedBuffer[MAX_CHAR]; //char array to store seed from seed.text
    int randSeed;              //the seed to be converted to int
    char slugFileNames[4][16] = {"seed_slug_1.txt", "seed_slug_2.txt", "seed_slug_3.txt", "seed_slug_4.txt"};
    printf("[Slug PID: %d] User entered: %d\n", pid, arg);
    printf("[Slug PID: %d] Reading from %s\n", pid, slugFileNames[arg - 1]); //-1 because index starts at 0!
    FILE *fp = fopen(slugFileNames[arg - 1], "r");                           //file pointer, set to read only

    //check if can't read file
    if (fp != NULL)
    {
        //I'm assuming there will only be one line here
        //for future implementations I'll learn how to only read
        //from the first line if only first line is necessary
        if (fgets(seedBuffer, MAX_CHAR, fp) != NULL)
        {
            fclose(fp);
            printf("[Slug PID: %d] Read seed value: %s\n", pid, seedBuffer);
            randSeed = atoi(seedBuffer);
            printf("[Slug PID: %d] Read seed value (converted to integer): %d\n", pid, randSeed);
            return randSeed;
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
}

int main(int argc, char *argv[])
{
    int pid = (int)getpid();                                 //process id
    int argNum;                                              //1, 2, 3, or 4, the argument user will enter
    int numSeconds;                                          //no. of seconds to wait before proceeding
    int randSeed;                                            //the seed to be converted to int
    int waitTime;                                            //no. of seconds to wait before proceeding
    int coinFlip;                                            //coinflip value
    char commands[2][20] = {"last -d --fulltimes", "id -u"}; //the command which will run based on 'coin flip'

    if (argc != 2) //if incorrect no. of arguments
    {
        printf("Incorrect number of arguments! This program expects a single whole number between 1 and 4.\n");
        exit(0);
    }
    else //correct no. of arguments
    {
        argNum = atoi(argv[1]);             //convert string input to integer
                                            //reading the man pages, atoi doesn't seem like a safe operation
                                            //if not required by the assignment, I would prefer using strtol
                                            //atoi will return 0 if string is an invalid int, and will truncate
                                            //characters superceding the int (eg. atoit(1abc) = 1).
        if ((argNum >= 1) && (argNum <= 4)) //If argument is a number between 1 and 4
        {
            randSeed = getSeed(argNum);
            waitTime = getRandom(randSeed, 1, 4);
            coinFlip = getRandom(randSeed, 0, 1);

            printf("[Slug PID: %d] Delay time is %d seconds. Flipping coin... %d\n", pid, waitTime, coinFlip);
            printf("[Slug PID: %d] I'll get the job done... Eventually...\n", pid);
            sleep(waitTime); //wait for randomly generated number of seconds before proceeding

            printf("[Slug PID: %d] Break time is over! I am running the '%s' command.\n", pid, commands[coinFlip]);

            //there most definitely is a cleaner way of doing this:
            if (coinFlip)
            {
                char *myArgs[3] = {"id", "-u", NULL};
                execvp(myArgs[0], myArgs);
            }
            else
            {
                char *myArgs[4] = {"last", "-d", "--fulltimes", NULL};
                execvp(myArgs[0], myArgs);
            }

            //for the above process, my original plan was to dynamically store the commands in the commands
            //variable to the myArgs array. however, I do not think the extra LOC or complication is worth it
            //as the above is a little more readable
        }
        else
        {
            printf("Please enter a whole number between 1 and 4 (inclusive) and try again.\n");
            exit(0);
        }
    }
    return 0;
}