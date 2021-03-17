#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "fedoops.h"

// stations
struct station stations[4];
package *pileHead = NULL; // pile of pending packages

// tracking packages completed
int packagesCompleted = 0;
int bluePackages = 0;
int redPackages = 0;
int greenPackages = 0;
int yellowPackages = 0;

// locks/mutexes
pthread_mutex_t grabMtx = PTHREAD_MUTEX_INITIALIZER;    // mutex for grabbing packages
pthread_mutex_t stationMtx = PTHREAD_MUTEX_INITIALIZER; // mutex for gettin onna station
pthread_mutex_t doneMtx = PTHREAD_MUTEX_INITIALIZER;

// condition variables for conveyor and worker queue
pthread_cond_t readyCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t teamConds[4] = {PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER};

void incrementPackages(int teamNum)
{
  // keeps track of the number of packages each team has completed globally
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

// main thread function, handles workers grabbing packages, doing work, moving packages, and joining the queue
void *slaveAway(void *arg)
{
  workerNode *worker = ((workerNode *)arg); // casting input to type worker
  package *workerPackage = NULL;            // just to prevent ptr->package->whatever
  station *currStation = NULL;              // pointer to station worker is working at or wants to move to

  int workerId = worker->workerId;                   // makes accessing the worker's id easier
  int workerTeam = worker->team;                     // makes accessing the worker's team number easier
  char *teamName = worker->teamName;                 // makes accessing the worker's team name easier
  pthread_cond_t *busyCond = &teamConds[workerTeam]; // setting the worker queue condition variable based on the worker's team

  while (1)
  {
    while (1)
    {

      if (pileHead == NULL)
      {
        return NULL; // terminate thread when there are no more packages
      }

      pthread_mutex_lock(&grabMtx);
      // grabbing a package should be protected
      if (worker->isFree)
      {
        break; // enter package grabbing logic if worker is free and package is available, grabMtx remains locked to make grabbing package atomic
      }
      else
      {
        printf("BUSY:  Currently not Worker %s #%d's turn\n",
               teamName, workerId);
        pthread_cond_wait(busyCond, &grabMtx); // if it is not worker's turn to grab a package, relinquish the CPU until a worker on the same team completes a package
        pthread_mutex_unlock(&grabMtx);
      }
    }

    worker->isFree = 0;              // once a worker grabs a package they cannot grab another
    worker->package = pileHead;      // store package into worker to work on
    workerPackage = worker->package; // store reference to package in workerPackage
    printf("GRAB:  Worker %s #%d grabbed package %d\n",
           teamName, workerId, workerPackage->packageNum); // prints info about worker id and package id when a worker grabs a package
    printf("INST:  Package #%d has a total of %d instructions\n",
           workerPackage->packageNum, workerPackage->instructionCount); // prints the number of instructions the given package has
    printf("INFO:  Package #%d has the following instructions: ", workerPackage->packageNum);
    printf("[");
    for (int i = 0; i < workerPackage->instructionCount; i++)
    // prints each of the instructions of the package in order
    {
      if (i)
        printf(", ");
      int temp;
      temp = workerPackage->custInstructions[i] - 1;
      printf("%s", temp == WEIGHT ? "Weight" : temp == BARCODE ? "Barcode"
                                           : temp == XRAY      ? "X-ray"
                                                               : "Jostle");
    }
    printf("]\n");
    pileHead = pileHead->nextPackage; // pile goes to next package for another worker to grab

    pthread_mutex_unlock(&grabMtx);

    printf("STRT:  Worker %s #%d is starting to work on package %d\n",
           teamName, workerId, workerPackage->packageNum); // prints that the worker is about to begin moving and working on packages

    for (int i = 0; i < workerPackage->instructionCount; i++) // cycles through each instruction for the given workerPackage
    {
      currStation = stations + (workerPackage->custInstructions[i] - 1);
      printf("MOVE:  Worker %s #%d is moving Package #%d to Station %s\n",
             teamName, workerId, workerPackage->packageNum, currStation->stationName); // prints that the worker is about to attempt to move a package, either from the PPP to a station or along the conveyor system

      /* below is the conveyor system */
      int stationFreeFlag = 1;
      while (1)
      {
        pthread_mutex_lock(&stationMtx); // checking whether or not a station is free must be atomic so no two workers and working at the same place at the same time
        stationFreeFlag = currStation->isFree;
        if (stationFreeFlag) // checks if the incoming station is free
        {
          usleep(rand() % (10000 - 1000 + 1) + 1000); // Conveyor belt - takes random time
          currStation->isFree = 0;                    // if a worker can move to a station, that station is no longer free
          pthread_mutex_unlock(&stationMtx);
          break; // enter working-on-package logic
        }
        else
        {
          printf("WAIT:  Station %s is currently busy. Worker %s #%d waiting with Package #%d\n",
                 currStation->stationName, teamName, workerId, workerPackage->packageNum); // prints that the worker cannot move to its desired station because the station is busy
          pthread_cond_wait(&readyCond, &stationMtx);                                      // waits for a worker to finish working on a station before checking station availability again
          pthread_mutex_unlock(&stationMtx);
        }
      }

      printf("WORK:  Worker %s #%d working on Package #%d at Station %s\n",
             teamName, workerId, workerPackage->packageNum, currStation->stationName); // prints out that a working is doing work at a station
      if (workerPackage->fragile == 1, strcmp(currStation->stationName, "Jostle") == 0)
      {
        printf("VLNT:  Package #%d is fragile! Worker %s #%d is shaking the sh*t out of it\n",
               workerPackage->packageNum, teamName, workerId); // shakes the package if necessary
      }

      usleep(rand() % (10000 - 1000 + 1) + 1000); // Work on package - random time

      printf("DONE:  Worker %s #%d is finished working on Package #%d at Station %s\n",
             teamName, workerId, workerPackage->packageNum, currStation->stationName); // prints that a worker finished working on a package at their station
      printf("FREE:  Station %s is now free\n", currStation->stationName);             // prints that the station they were previously working on is now free
      currStation->isFree = 1;                                                         // sets the current station to free, simulating the working picking up their package from the station
      pthread_cond_broadcast(&readyCond);                                              // broadcasts the condition variable to every worker currently waiting on a station, allowing them to recheck if they can move where they want to go
    }
    worker->nextWorker->isFree = 1; // lets the next worker in the queue know that it can now grab a package
    pthread_mutex_lock(&doneMtx);   // incrementing packages must be atomic
    incrementPackages(workerTeam);  // calls package incrementing helper function
    pthread_mutex_unlock(&doneMtx); // grabbing a package should be protected
    printf("CMLT:  Worker %s #%d is finished working on Package #%d \n",
           teamName, workerId, workerPackage->packageNum); // prints that a worker is finished working on their package
    pthread_cond_broadcast(busyCond);                      // lets the workers in the worker's team's queue know that they might be next in line
  }
}

int main()
{
  workerNode *workersHead[4] = {NULL, NULL, NULL, NULL}; // initializes the worker queue heads

  int randSeed = getSeed(); // calls helper function to extract seed from seed.txt
  printf("Random seed is: %d\n", randSeed);
  sleep(1);
  printf("Seeding the randomizer...\n");
  sleep(1);
  srand(randSeed); // seed the randomizer

  // number of packageCount, randomly generated (current upper and lower bounds are 80 to adhere to project specs)
  int packageCount = rand() % (UPPER - LOWER + 1) + LOWER;
  printf("Total number of packages to be processed is: %d\n", packageCount);
  // sleep(1);

  // initialize everything
  createPackages(packageCount, &pileHead); // calls helper function to create PPP
  createStations(stations);                // calls helper function to create each station

  for (int i = 0; i < NUM_TEAMS; i++)
  {
    createWorkers(&workersHead[i], i); // creates each worker queue and stores 10 workers in each
  }

  pthread_t *workerThreads = (pthread_t *)malloc(NUM_WORKERS * NUM_TEAMS * sizeof(pthread_t)); // allocating enough memory for each pthread (number of workers per team * number of teams * size of pthread)

  // spawn threads (40 threads, 4 teams 10 workers each team)
  printf("Ready... get set... slave you corporate slaves :^)\n");
  // sleep(1);

  // printWorkers(&workersHead[0]);
  int spawnIndex = 0;                 // keeps track of which thread is being spawned
  for (int i = 0; i < NUM_TEAMS; i++) // loops through each team
  {
    for (int j = 0; j < NUM_WORKERS; j++) // loops through each worker in each team
    {
      pthread_create(&workerThreads[spawnIndex], NULL, slaveAway, workersHead[i]);
      workersHead[i] = workersHead[i]->nextWorker;
      spawnIndex++;
    }
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
