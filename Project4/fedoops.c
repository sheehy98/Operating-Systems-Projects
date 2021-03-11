#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include "fedoops.h"

// stations
struct station stations[4];
int intStations[4] = {0, 0, 0, 0}; // wouldn't it be easier to do it this way?
package *pileHead = NULL;          // pile of pending packages

void createStations()
{
    for (int i = 0; i < 4; i++)
    {
        stations[i].isFree = 1;
    }

    return;
}

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

// if randomly generated no. of packages is > 40, we need to figure out
// how to loop back in and finish the packages
void *slaveAway(void *arg)
{
    workerNode *workerHead = ((workerNode *)arg);
    workerNode *worker = workerHead;
    package *workerPackage = NULL; // just to prevent ptr->package->whatever

    int workerId = worker->workerId;
    int workerTeam = worker->team;

    // grab package
    if (pileHead->ready && worker->isFree)
    {
        pileHead->ready = 0;
        worker->isFree = 0;
        worker->package = pileHead;
        workerPackage = worker->package;
        pileHead = pileHead->nextPackage; // pile goes to next package for another worker to grab
        printf("Worker %d from team %d has grabbed package %d\n", workerId, workerTeam, workerPackage->packageNum);
    }

    return NULL;
}

int main()
{
    workerNode *workersHead[4] = {NULL, NULL, NULL, NULL};

    // int packageCount = 0;
    int randSeed = getSeed();
    printf("Random seed is: %d\n", randSeed);
    printf("Seeding the randomizer...\n");
    srand(randSeed); // seed the randomizer

    // number of packageCount, randomly generated
    int packageCount = rand() % (UPPER - LOWER + 1) + LOWER;
    printf("Total number of packages to be processed is: %d\n", packageCount);

    // initialize everything
    createPackages(packageCount, &pileHead);
    createStations();

    for (int i = 0; i < NUM_TEAMS; i++)
    {
        createWorkers(&workersHead[i], i);
    }

    pthread_t *workerThreads = (pthread_t *)malloc(NUM_WORKERS * NUM_TEAMS * sizeof(pthread_t));

    // spawn threads (40 threads, 4 teams 10 workers each team)
    int teamIndex = 0;
    for (int i = 0; i < NUM_TEAMS; i++)
    {
        for (int j = 0; j < NUM_WORKERS; j++)
        {
            pthread_create(&workerThreads[teamIndex * j], NULL, slaveAway, workersHead[i]);
            workersHead[i] = workersHead[i]->nextWorker;
        }
        teamIndex++;
    }

    // join threads
    int joinIndex = 0;
    for (int i = 0; i < NUM_TEAMS; i++)
    {
        for (int j = 0; j < NUM_WORKERS; j++)
        {
            pthread_join(workerThreads[joinIndex], NULL);
            joinIndex++;
        }
    }

    printf("joined\n");

    free(workerThreads);

    return 0;
}
