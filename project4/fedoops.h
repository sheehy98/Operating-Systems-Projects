#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
#define UPPER 80
#define LOWER 80

// num workers
#define NUM_TEAMS 4
#define NUM_WORKERS 10

typedef struct package
{
    int packageNum;
    int instructionCount;
    int fragile;
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
    // struct workerNode *lastWorker;
} workerNode;

typedef struct station
{
  int isFree; // if the station is currently being worked at
  char *stationName; // the station's name, useful for console printouts
} station;

void appendWorker(struct workerNode **head, int workerId, int team);
void appendPackage(struct package **head, int packageNum, int instructionCount);
void createPackages(int packageCount, struct package **head);

// initializes the stations
void createStations(struct station *stations) 
{
    for (int i = 0; i < 4; i++)
    {
        stations[i].isFree = 1;
        stations[i].stationName = i == WEIGHT ? "Weight" : i == BARCODE ? "Barcode"
                                                       : i == XRAY      ? "X-ray"
                                                                        : "Jostle";
    }

    return;
}

//deprecated
void printPackages(struct package **head)
{
    struct package *ptr = *head;

    while (ptr != NULL)
    {
        printf("Package #%d, Fragile = %d,  with no. instructs %d \n", ptr->packageNum, ptr->fragile, ptr->instructionCount);
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

// initializes the list of packages
void createPackages(int packageCount, struct package **head)
{

    int instructionCount = 1;

    for (int i = 0; i < packageCount; i++)
    {
      instructionCount = (rand() % 4) + 1;
      appendPackage(head, i + 1, instructionCount);
    }

    return;
}

// initilizes a team of workers
void createWorkers(workerNode **head, int i)
{
    for (int j = 0; j < NUM_WORKERS + 1; j++)
    {
        appendWorker(head, j, i);
    }
}

// deprecated
void printWorkers(workerNode **head)
{
    workerNode *ptr = *head;

    while (ptr != NULL)
    {
        printf("Worker #%d on team %d, FREE = %d\n", ptr->workerId, ptr->team, ptr->isFree);
        ptr = ptr->nextWorker;
    }
}

// creates the random list of instructions for a given package
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

// adds a worker to a worker queue
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
    new->isFree = workerId == 0 ? 1 : 0;

    /* will be last node */
    new->nextWorker = NULL;

    if (*head == NULL)
    {
        *head = new;
        return;
    }

    while (last->nextWorker != NULL)
        last = last->nextWorker;

    if (workerId == (NUM_WORKERS))
    {
        last->nextWorker = *head;
    }
    else
    {
        last->nextWorker = new;
    }

    return;
}

// adds a package to the PPP
void appendPackage(struct package **head, int packageNum, int instructionCount)
{
    struct package *new = (struct package *)malloc(sizeof(struct package));
    struct package *last = *head;

    /* add data */
    new->packageNum = packageNum;
    new->instructionCount = instructionCount;
    new->ready = 1;
    new->fragile = rand() % 2;
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

// grabs the seed from seed.txt
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

