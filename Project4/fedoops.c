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

int pkgCmplt = 0;

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

    while (1)
    {
        // grab package
        pthread_mutex_lock(&grabMtx); // grabbing a package should be protected
        if (pileHead != NULL)
        {
            pileHead->ready = 0;
            worker->isFree = 0;
            worker->package = pileHead;      // store package into worker to work on
            workerPackage = worker->package; // store reference to package in workerPackage
            isPackageGrabbed = 1;
            printf("GRAB:  Worker %s #%d grabbed package %d\n", teamName, workerId, workerPackage->packageNum);
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
            printf("MOVE:  Worker %s #%d is moving Package #%d to %s\n",
                   teamName, workerId, workerPackage->packageNum, currStation->stationName);
            int sOpen = 1;
            while (1)
            {
                pthread_mutex_lock(&stationMtx);
                sOpen = currStation->isFree;
                if (sOpen)
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

            printf("WORK:  Worker %s #%d working on Package #%d at Station %s\n",
                   teamName, workerId, workerPackage->packageNum, currStation->stationName);

            pthread_mutex_lock(&stationMtx);
            usleep(1000); //Doing the work of a station
            pthread_mutex_unlock(&stationMtx);

            printf("DONE:  Worker %s #%d is finished working on Package #%d at Station %s\n",
                   teamName, workerId, workerPackage->packageNum, currStation->stationName);
            printf("FREE:  Station %s is now free\n", currStation->stationName);
            currStation->isFree = 1;

            pthread_cond_signal(&cond) == -1;
        }
        pkgCmplt++;
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
    createStations();

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

    printf("Packages requested %d, packages completed %d \n", packageCount, pkgCmplt);

    printf("Well done peasants. Shift is over :)\n");

    free(workerThreads);

    return 0;
}