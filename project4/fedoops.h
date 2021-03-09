#include <stdio.h>
#include <stdlib.h>
#define MAX_CHAR 255

#define FALSE 0
#define TRUE 1

#define BLUE 0
#define RED 1
#define GREEN 2
#define YELLOW 3

// the jobs, also stations
#define WEIGHT 0
#define BARCODE 1
#define XRAY 2
#define JOSTLE 3

// package limits
#define UPPER 100
#define LOWER 40

// num workers
#define NUM_TEAMS 4
#define NUM_WORKERS 10

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
    int workerId;
    int team;                  // the team
    int isFree;                // is the worker ready to accept a package
    struct package *package;   // the package the worker is working
    struct worker *nextWorker; // the next worker in queue
    // struct worker *prevWorker; // the previous worker in queue
} worker;

typedef struct station
{
    int isFree;
} station;

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

void appendWorker(struct worker **head, int workerId, int team)
{
    worker *new = (worker *)malloc(sizeof(worker));
    worker *last = *head;

    /* add data */
    new->workerId = workerId + 1;
    new->team = team;
    new->package = (package *)malloc(sizeof(package));
    new->isFree = 1;

    /* will be last node */
    new->nextWorker = NULL;

    if (*head == NULL)
    {
        *head = new;
        return;
    }

    while (last->nextWorker != NULL)
        last = last->nextWorker;

    last->nextWorker = new;
    return;
}

void appendPackage(struct package **head, int packageNum, int instructionCount)
{
    struct package *new = (struct package *)malloc(sizeof(struct package));
    struct package *last = *head;

    /* add data */
    new->packageNum = packageNum;
    new->instructionCount = instructionCount;
    new->ready = 1;
    new->currStation = 0;
    generateJobs(&new, instructionCount);

    /* will be last node */
    new->nextPackage = NULL;

    if (*head == NULL)
    {
        *head = new;
        return;
    }

    while (last->nextPackage != NULL)
        last = last->nextPackage;

    last->nextPackage = new;
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
