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

typedef struct workerNode
{
    int workerId;
    int team;                      // the team
    char *teamName;                // the team name
    int isFree;                    // is the worker ready to accept a package
    struct package *package;       // the package the worker is working
    struct workerNode *nextWorker; // the next worker in queue
    // struct worker *prevWorker; // the previous worker in queue
} workerNode;

void appendWorker(struct workerNode **head, int workerId, int team);
void appendPackage(struct package **head, int packageNum, int instructionCount);
void createPackages(int packageCount, struct package **head);

typedef struct station
{
    int isFree;
    char *stationName;
} station;

void printPackages(struct package **head)
{
    struct package *ptr = *head;

    while (ptr != NULL)
    {
        printf("Package #%d with no. instructs %d \n", ptr->packageNum, ptr->instructionCount);
        printf("\tInstructions array = ");
        printf("[");
        for (int i = 0; i < ptr->instructionCount; i++)
        {
            if (i)
                printf(", ");
            printf("%d", ptr->custInstructions[i]);
        }
        printf("]\n");
        ptr = ptr->nextPackage;
    }
}

void createPackages(int packageCount, struct package **head)
{

    int instructionCount = 0;

    for (int i = 0; i < packageCount; i++)
    {
        instructionCount = (rand() % 4) + 1;
        appendPackage(head, i + 1, instructionCount);
    }

    return;
}

void createWorkers(workerNode **head, int i)
{
    for (int j = 0; j < NUM_WORKERS; j++)
    {
        appendWorker(head, j, i);
    }
}

void printWorkers(workerNode **head)
{
    workerNode *ptr = *head;

    while (ptr != NULL)
    {
        printf("Worker #%d on team %d \n", ptr->workerId, ptr->team);
        ptr = ptr->nextWorker;
    }
}

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

void appendWorker(struct workerNode **head, int workerId, int team)
{
    workerNode *new = (workerNode *)malloc(sizeof(workerNode));
    workerNode *last = *head;

    /* add data */
    new->workerId = workerId + 1;
    new->team = team;
    new->teamName =
        team == 0 ? "Blue" : team == 1 ? "Red"
                         : team == 2   ? "Green"
                                       : "Yellow";
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
