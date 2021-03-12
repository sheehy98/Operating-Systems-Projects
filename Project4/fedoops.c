#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "fedoops.h"

// CAN ONLY HAVE ONE WORKER PER TEAM AT A TIME

// stations
struct station stations[4];
package *pileHead = NULL; // pile of pending packages

//
int teamBusy[4] = {0, 0, 0, 0};

// tracking packages completed
int packagesCompleted = 0;
int bluePackages = 0;
int redPackages = 0;
int greenPackages = 0;
int yellowPackages = 0;

// locks/mutexes
pthread_mutex_t grabMtx = PTHREAD_MUTEX_INITIALIZER;    // mutex for grabbing packages
pthread_mutex_t stationMtx = PTHREAD_MUTEX_INITIALIZER; // mutex for gettin onna station
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;         // conditional :)

void incrementPackages(int teamNum)
{
    packagesCompleted++;

    switch (teamNum)
    {
    case 0:
        bluePackages++;
        break;
    case 1:
        redPackages++;
        break;
    case 2:
        greenPackages++;
        break;
    case 3:
        yellowPackages++;
        break;
    }

    return;
}

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

    while (1)
    {
        // grab package
        pthread_mutex_lock(&grabMtx); // grabbing a package should be protected
        if (teamBusy[workerTeam])
        {

            printf("BUSY:  Team %s already has a worker working. Worker %s #%d waiting\n",
                   teamName, teamName, workerId);
            pthread_cond_wait(&cond, &grabMtx);
        }

        if (pileHead != NULL)
        {
            teamBusy[workerTeam] = 1;
            pileHead->ready = 0;
            worker->isFree = 0;
            worker->package = pileHead;      // store package into worker to work on
            workerPackage = worker->package; // store reference to package in workerPackage
            isPackageGrabbed = 1;
            printf("GRAB:  Worker %s #%d grabbed package %d\n",
                   teamName, workerId, workerPackage->packageNum);
            pileHead = pileHead->nextPackage; // pile goes to next package for another worker to grab
        }
        else
        {
            pthread_mutex_unlock(&grabMtx);
            break;
        }
        pthread_mutex_unlock(&grabMtx);

        printf("STRT:  Worker %s #%d is starting to work on package %d\n",
               teamName, workerId, workerPackage->packageNum);
        printf("INST:  Package #%d has a total of %d instructions\n",
               workerPackage->packageNum, workerPackage->instructionCount);

        for (int i = 0; i < workerPackage->instructionCount; i++)
        {
            currStation = stations + (workerPackage->custInstructions[i] - 1);
            printf("MOVE:  Worker %s #%d is moving Package #%d to Station %s\n",
                   teamName, workerId, workerPackage->packageNum, currStation->stationName);

            /* below is the conveyor system */
            int stationFreeFlag = 1;
            while (1)
            {
                pthread_mutex_lock(&stationMtx);
                stationFreeFlag = currStation->isFree;
                if (stationFreeFlag)
                {
                    currStation->isFree = 0;
                    pthread_mutex_unlock(&stationMtx);
                    break;
                }
                else
                {
                    printf("WAIT:  Station %s is currently busy. Worker %s #%d waiting with Package #%d\n",
                           currStation->stationName, teamName, workerId, workerPackage->packageNum);
                    pthread_cond_wait(&cond, &stationMtx);
                    pthread_mutex_unlock(&stationMtx);
                }
            }

            pthread_mutex_lock(&stationMtx);
            printf("WORK:  Worker %s #%d working on Package #%d at Station %s\n",
                   teamName, workerId, workerPackage->packageNum, currStation->stationName);

            if (workerPackage->fragile == 1, strcmp(currStation->stationName, "Jostle") == 0)
            {
                printf("VLNT:  Package #%d is fragile! Worker %s #%d is shaking the sh*t out of it\n",
                       workerPackage->packageNum, teamName, workerId);
            }

            usleep(1000); //Doing the work of a station

            printf("DONE:  Worker %s #%d is finished working on Package #%d at Station %s\n",
                   teamName, workerId, workerPackage->packageNum, currStation->stationName);
            printf("FREE:  Station %s is now free\n", currStation->stationName);
            currStation->isFree = 1;

            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&stationMtx);
        }

        teamBusy[workerTeam] = 0;
        incrementPackages(workerTeam);
        printf("CMLT:  Worker %s #%d is finished working on Package #%d \n",
               teamName, workerId, workerPackage->packageNum);
    }
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
    createStations(stations);

    for (int i = 0; i < NUM_TEAMS; i++)
    {
        createWorkers(&workersHead[i], i);
    }

    pthread_t *workerThreads = (pthread_t *)malloc(NUM_WORKERS * NUM_TEAMS * sizeof(pthread_t));

    // spawn threads (40 threads, 4 teams 10 workers each team)
    printf("Ready... get set... slave you corporate slaves :^)\n");
    // sleep(1);

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

    printf("----------------------------------------------------------------\n");
    printf("Packages requested: %d\tPackages completed: %d \n", packageCount, packagesCompleted);
    printf("Blue Processed: %d\tRed Processed: %d\tGreen Processed: %d\tYellow Processed: %d\n",
           bluePackages, redPackages, greenPackages, yellowPackages);

    printf("Well done peasants. Shift is over :)\n");

    free(workerThreads);

    return 0;
}