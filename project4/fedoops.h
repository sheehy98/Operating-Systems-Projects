#include <stdio.h>
#include <stdlib.h>
#define MAX_CHAR 255

#define FALSE 0
#define TRUE 1

#define BLUE 1
#define RED 2
#define GREEN 3
#define YELLOW 4

// the jobs, also stations
#define WEIGHT 1
#define BARCODE 2
#define XRAY 3
#define JOSTLE 4

// package limits
#define UPPER 50
#define LOWER 5

typedef struct package
{
    int packageNum;
    int instructionCount;
    int custInstructions[4];     // array of jobs to be completed for the package
    int currStation;             // the current station the package is on
    int ready;                   // bool, ready to go to next station or not
    struct package *nextPackage; // next package in queue
} package;

typedef struct worker
{
    int team;                  // the team
    int isFree;                // is the worker ready to accept a package
    struct package *package;   // the package the worker is working on
    struct worker *nextWorker; // the next worker in queue
    struct worker *prevWorker; // the previous worker in queue
} worker;

void generateJobs(struct package **node, int instructionCount)
{
    int j = 0;
    struct package *ptr = *node;

    while (j < instructionCount)
    {
        int flag = 0; // flag for duplicate value
        int instruction = (rand() % 4) + 1;

        for (int k = 0; k < j; k++)
        {
            if (instruction == ptr->custInstructions[k])
            {
                flag = 1;
                break;
            }
        }

        if (flag != 1)
        {
            ptr->custInstructions[j] = instruction;
            j++;
        }
    }
}

void appendPackage(struct package **head, int packageNum, int instructionCount)
{
    struct package *link = (struct package *)malloc(sizeof(struct package));
    struct package *last = *head;

    /* add data */
    link->packageNum = packageNum;
    link->instructionCount = instructionCount;
    link->ready = 1;
    link->currStation = 0;
    generateJobs(&link, instructionCount);

    /* will be last node */
    link->nextPackage = NULL;

    if (*head = NULL)
    {
        *head = link;
        return;
    }

    while (last->nextPackage != NULL)
        last = last->nextPackage;

    last->nextPackage = link;
    return;
}

int getSeed()
{
    char seedBuffer[MAX_CHAR];         // char array to store seed from seed.text
    int randSeed;                      // the seed to be converted to int
    FILE *fp = fopen("seed.txt", "r"); // file pointer, set to read only

    // check if can't read file
    if (fp != NULL)
    {
        if (fgets(seedBuffer, MAX_CHAR, fp) != NULL)
        {
            fclose(fp);
            randSeed = atoi(seedBuffer);
            return randSeed;
        }
        else
        {
            printf("Seed file is empty! Exiting...\n");
            exit(1);
        }
    }
    else
    {
        printf("Cannot read from seed file! Exiting...\n");
        exit(1);
    }
}
