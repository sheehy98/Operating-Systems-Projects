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

// locks/mutexes

pthread_mutex_t grabMtx = PTHREAD_MUTEX_INITIALIZER;     // mutex for grabbing packages
pthread_mutex_t stationMtx = PTHREAD_MUTEX_INITIALIZER;  // mutex for gettin onna station
pthread_mutex_t conveyorMtx = PTHREAD_MUTEX_INITIALIZER; // mutex for gettin onna conveyor
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void createStations()
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

/* deprecated */
void printGrabbed(workerNode *head)
{
    int team = head->team;
    int workerId = head->workerId;
    int packageNum = head->package->packageNum;

    switch (team)
    {
    case (0):
        printf("Worker #%d from Team Blue has grabbed package #%d\n", workerId, packageNum);
        break;
    case (1):
        printf("Worker #%d from Team Red has grabbed package #%d\n", workerId, packageNum);
        break;
    case (2):
        printf("Worker #%d from Team Green has grabbed package #%d\n", workerId, packageNum);
        break;
    case (3):
        printf("Worker #%d from Team Yellow has grabbed package #%d\n", workerId, packageNum);
        break;
    }
}

// if randomly generated no. of packages is > 40, we need to figure out
// how to loop back in and finish the packages
void *slaveAway(void *arg)
{
    workerNode *workerHead = ((workerNode *)arg);
    workerNode *worker = workerHead;
    package *workerPackage = NULL; // just to prevent ptr->package->whatever
    station *currStation = NULL;
    int isPackageGrabbed = 0; // flag to check if package is grabbed
    int workerWaiting = 0;

    int workerId = worker->workerId;
    int workerTeam = worker->team;
    char *teamName = worker->teamName;

    // while (pileHead != NULL)
    // {

    // grab package
    pthread_mutex_lock(&grabMtx); // grabbing a package should be protected
    if (pileHead->ready && worker->isFree)
    {
        pileHead->ready = 0;
        worker->isFree = 0;
        worker->package = pileHead;      // store package into worker to work on
        workerPackage = worker->package; // store reference to package in workerPackage
        isPackageGrabbed = 1;
        printf("GRAB:  Worker %s #%d grabbed package %d\n", teamName, workerId, workerPackage->packageNum);
        pileHead = pileHead->nextPackage; // pile goes to next package for another worker to grab
    }
    pthread_mutex_unlock(&grabMtx);

    if (isPackageGrabbed)
    {
        printf("STRT:  Worker %s #%d is starting to work on package %d\n",
               teamName, workerId, workerPackage->packageNum);
        printf("INST:  Package #%d has a total of %d instructions\n",
               workerPackage->packageNum, workerPackage->instructionCount);

        for (int i = 0; i < workerPackage->instructionCount; i++)
        {
            currStation = stations + (workerPackage->custInstructions[i] - 1);
            printf("MOVE:  Worker %s #%d is moving Package #%d to %s\n",
                   teamName, workerId, workerPackage->packageNum, currStation->stationName);

            int isFree = 1;

            pthread_mutex_lock(&stationMtx); // grabbing a package should be protected
            isFree = currStation->isFree;
            pthread_mutex_unlock(&stationMtx); // grabbing a package should be protected

            if (!isFree)
            {
                workerWaiting = 1;
                printf("WAIT:  Station %s is currently busy. Worker %s #%d waiting with Package #%d\n",
                       currStation->stationName, teamName, workerId, workerPackage->packageNum);
                pthread_cond_wait(&cond, &stationMtx);

                pthread_mutex_lock(&stationMtx); // grabbing a package should be protected
                isFree = currStation->isFree;
                pthread_mutex_unlock(&stationMtx); // grabbing a package should be protected
            }

            // if (isFree)
            // {
            if (workerWaiting)
            {
                printf("WAIT:  Station %s is now free. Worker %s #%d is moving Package #%d\n",
                       currStation->stationName, teamName, workerId, workerPackage->packageNum);
            }

            pthread_mutex_lock(&stationMtx); // grabbing a package should be protected
            currStation->isFree = 0;
            pthread_mutex_unlock(&stationMtx); // grabbing a package should be protected

            printf("WORK:  Worker %s #%d working on Package #%d at Station %s\n",
                   teamName, workerId, workerPackage->packageNum, currStation->stationName);
            sleep(1);
            // pthread_mutex_lock(&stationMtx); // grabbing a package should be protected
            printf("FREE:  Station %s is now free\n", currStation->stationName);
            currStation->isFree = 1;
            // pthread_mutex_unlock(&stationMtx); // grabbing a package should be protected
            // }

            pthread_cond_signal(&cond);
        }

        // printf("WORK:  Worker %s #%d working on Package #%d at Station %s\n",
        //        teamName, workerId, workerPackage->packageNum, currStation->stationName);

        // sleep(1);
    }
    // }

    return NULL;
}

int main()
{
    workerNode *workersHead[4] = {NULL, NULL, NULL, NULL};

    // int packageCount = 0;
    int randSeed = getSeed();
    printf("Random seed is: %d\n", randSeed);
    // sleep(1);
    printf("Seeding the randomizer...\n");
    // sleep(1);
    srand(randSeed); // seed the randomizer

    // number of packageCount, randomly generated
    int packageCount = rand() % (UPPER - LOWER + 1) + LOWER;
    printf("Total number of packages to be processed is: %d\n", packageCount);
    // sleep(1);

    // initialize everything
    createPackages(packageCount, &pileHead);
    createStations();

    for (int i = 0; i < NUM_TEAMS; i++)
    {
        createWorkers(&workersHead[i], i);
    }

    pthread_t *workerThreads = (pthread_t *)malloc(NUM_WORKERS * NUM_TEAMS * sizeof(pthread_t));

    // spawn threads (40 threads, 4 teams 10 workers each team)
    printf("Ready... get set... slave you corporate slaves :^)\n");
    sleep(1);

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

    printf("Well done peasants. Shift is over :)\n");

    free(workerThreads);

    return 0;
}
