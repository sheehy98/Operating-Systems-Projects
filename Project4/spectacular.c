#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include "spectacular.h"

// Three performer types:
// Put in structs: juggler, flamenco dancer, soloists
// Stage:
// could be a struct with a name?

// Rules:
// Flamenco and jugglers cannot be on stage at the same time
// Soloists must be on stage alone (one soloists on stage at a time)
// However, multiple flamencos and jugglers on stage at a time
// Only four performers on stage at a time
// Random execution times (how long each perfomer performs for) - upper bounded
// No busy wait?

sem_t stageSem, printSem, leaveStageSem, enterSem;

struct performer jugglers[JUGGLER_TOT] = {NULL};
struct performer flamencos[FLAMENCO_TOT] = {NULL};
struct performer soloists[SOLOISTS_TOT] = {NULL};

struct stage performanceStage;

int currType = 0;

void createStage()
{
    performanceStage.onStageTotal = 0;
    for (int i = 0; i < 4; i++)
    {
        char *positionName = i == 0 ? "Position Zero" : i == 1 ? "Position One"
                                                    : i == 2   ? "Position Two"
                                                               : "Position Three";
        performanceStage.stagePositions[i].currPerformer = NULL;
        performanceStage.stagePositions[i].isFree = 1;
        performanceStage.stagePositions[i].positionName = positionName;
    }
}

void printStage()
{
    printf("There are currently %d performers on stage\n", performanceStage.onStageTotal);
    printf("[ ");
    for (int i = 0; i < 4; i++)
    {
        if (performanceStage.stagePositions[i].currPerformer != NULL)
        {
            if (performanceStage.stagePositions[i].currPerformer->id < 10)
            {

                printf("%s #0%d ", performanceStage.stagePositions[i].currPerformer->typeName, performanceStage.stagePositions[i].currPerformer->id);
            }
            else
            {
                printf("%s #%d ", performanceStage.stagePositions[i].currPerformer->typeName, performanceStage.stagePositions[i].currPerformer->id);
            }
        }
        else
        {
            printf("________ ");
        }
    }
    printf("]\n\n");
}

void createPerformers()
{
    for (int i = 0; i < JUGGLER_TOT; i++)
    {
        jugglers[i].id = i + 1;
        jugglers[i].typeName = "JGLR";
        jugglers[i].performType = JUGGLER;
    }

    for (int i = 0; i < FLAMENCO_TOT; i++)
    {
        flamencos[i].id = i + 1;
        flamencos[i].typeName = "FDAN";
        flamencos[i].performType = FLAMENCO;
    }

    for (int i = 0; i < SOLOISTS_TOT; i++)
    {
        soloists[i].id = i + 1;
        soloists[i].typeName = "SOLO";
        soloists[i].performType = SOLOIST;
    }
}

void printPerformers(int performerType)
{
    switch (performerType)
    {
    case 0:
        for (int i = 0; i < JUGGLER_TOT; i++)
        {
            printf("%s #%d %d\n", jugglers[i].typeName, jugglers[i].id, jugglers[i].performType);
        }
        break;
    case 1:
        for (int i = 0; i < FLAMENCO_TOT; i++)
        {
            printf("%s #%d %d\n", flamencos[i].typeName, flamencos[i].id, flamencos[i].performType);
        }
        break;
    case 2:
        for (int i = 0; i < SOLOISTS_TOT; i++)
        {
            printf("%s #%d %d\n", soloists[i].typeName, soloists[i].id, soloists[i].performType);
        }
        break;
    }
}

void perform()
{
}

void *performerFunction(void *arg)
{
    struct performer *currPerformer = ((struct performer *)arg);
    char *performerTypeName = currPerformer->typeName;
    int performerTypeInt = currPerformer->performType;
    int performerId = currPerformer->id;
    char *positionName;
    int randPerformDuration = rand() % (10000 - 1000 + 1) + 1000; // performance duration btw 1ms to 10ms
    int stageIndex = 0;

    // if (performerTypeInt == 2)
    // {
    //     sem_wait(&stageSem);
    //     sem_wait(&stageSem);
    //     sem_wait(&stageSem);
    // }

    sem_wait(&stageSem);
    while (1)
    {
        sem_wait(&enterSem);
        if (performanceStage.onStageTotal == 0)
        {
            currType = performerTypeInt;
        }

        if (performerTypeInt == currType)
        {
            if (!(currType == 2 && performanceStage.onStageTotal != 0))
            {
                break;
            }
        }
        sem_post(&enterSem);
    }

    for (int i = 0; i < 4; i++)
    {
        if (performanceStage.stagePositions[i].isFree)
        {
            performanceStage.stagePositions[i].isFree = 0;
            performanceStage.stagePositions[i].currPerformer = currPerformer;
            performanceStage.onStageTotal++;
            positionName = performanceStage.stagePositions[i].positionName;
            stageIndex = i;
            break;
        }
    }

    sem_wait(&printSem);

    printf("PRFM:  %s #%d at %s performing for %d milliseconds\n",
           performerTypeName, performerId, positionName, randPerformDuration);
    printStage();

    sem_post(&printSem);

    sem_post(&enterSem);

    usleep(randPerformDuration);

    sem_wait(&leaveStageSem);

    performanceStage.stagePositions[stageIndex].isFree = 1;
    performanceStage.stagePositions[stageIndex].currPerformer = NULL;
    performanceStage.onStageTotal--;

    sem_wait(&printSem);

    // printf("DONE:  %s #%d is done performing at %s \n",
    //        performerTypeName, performerId, positionName);
    // printStage();

    sem_post(&printSem);

    sem_post(&leaveStageSem);

    // if (performerTypeInt == 2)
    // {
    //     sem_post(&stageSem);
    //     sem_post(&stageSem);
    //     sem_post(&stageSem);
    // }
    sem_post(&stageSem);
}

void createThreads(pthread_t *threads, int PERFORMER_TOTAL, struct performer *performers)
{
    for (int i = 0; i < PERFORMER_TOTAL; i++)
    {
        pthread_create(&threads[i], NULL, performerFunction, &performers[i]);
    }
}

void joinThreads(pthread_t *threads, int PERFORMER_TOTAL)
{
    for (int i = 0; i < PERFORMER_TOTAL; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

int main()
{
    int randSeed = getSeed();
    printf("Random seed is: %d\n", randSeed);
    // sleep(1);
    printf("Seeding the randomizer...\n");
    // sleep(1);
    srand(randSeed); // seed the randomizer

    sem_init(&stageSem, 0, 4);
    sem_init(&leaveStageSem, 0, 1);
    sem_init(&enterSem, 0, 1);
    sem_init(&printSem, 0, 1);

    printf("---------------------------------------------------------------\n");
    printf("Welcome to teh summer spectacular. take ur seats. bye\n");
    printf("---------------------------------------------------------------\n\n");

    // pthread_t jugglerThreads[JUGGLER_TOT], flamenco[]
    pthread_t *jugglerThreads = (pthread_t *)malloc(JUGGLER_TOT * sizeof(pthread_t));
    pthread_t *flamencoThreads = (pthread_t *)malloc(FLAMENCO_TOT * sizeof(pthread_t));
    pthread_t *soloistThreads = (pthread_t *)malloc(SOLOISTS_TOT * sizeof(pthread_t));

    createStage();
    createPerformers();

    createThreads(jugglerThreads, JUGGLER_TOT, jugglers);
    createThreads(flamencoThreads, FLAMENCO_TOT, flamencos);
    createThreads(soloistThreads, SOLOISTS_TOT, soloists);

    joinThreads(jugglerThreads, JUGGLER_TOT);
    joinThreads(flamencoThreads, FLAMENCO_TOT);
    joinThreads(soloistThreads, SOLOISTS_TOT);

    printf("---------------------------------------------------------------\n");
    printf("woohoo its over yay\n");
    printf("---------------------------------------------------------------\n");

    return 0;
}