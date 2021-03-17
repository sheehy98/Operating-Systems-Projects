#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include "spectacular.h"

sem_t stageSem, printSem, leaveStageSem, enterSem, napSem;
sem_t stageLock, fdancerLock, soloistLock, jugglerLock, emptySem, orderSem;

struct performer jugglers[JUGGLER_TOT] = {NULL};
struct performer flamencos[FLAMENCO_TOT] = {NULL};
struct performer soloists[SOLOISTS_TOT] = {NULL};

struct stage performanceStage;

int currType = 0;

int totalfDancers, totalJugglers;

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
    // printf("There are currently %d performers on stage\n", performanceStage.onStageTotal);
    // printf("[ __ZERO__ __ONE___ POS__TWO POS_THREE")
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
    printf("] ");
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

void *performerFunction(void *arg)
{
    struct performer *currPerformer = ((struct performer *)arg);
    char *performerTypeName = currPerformer->typeName;
    int performerTypeInt = currPerformer->performType;
    int performerId = currPerformer->id;
    char *positionName;
    int randPerformDuration = rand() % (10000 - 1000 + 1) + 1000; // performance duration btw 1ms to 10ms
    int stageIndex = 0;
    int isCorrect = 0;

    sem_t *lock = performerTypeInt == 0 ? &jugglerLock : performerTypeInt == 1 ? &fdancerLock
                                                                               : &soloistLock;

    if (performerTypeInt < 2)
    {
        sem_wait(&orderSem);
        sem_wait(lock);
        int *totalPerformers = performerTypeInt == 0 ? &totalfDancers : &totalJugglers;
        (*totalPerformers)++;
        if (*totalPerformers == 1)
        {
            sem_wait(&emptySem);
        }
        sem_post(&orderSem);
        sem_post(lock);

        sem_wait(&stageSem);
        sem_wait(&stageLock);
        for (int i = 0; i < 4; i++)
        {
            if (performanceStage.stagePositions[i].isFree)
            {
                performanceStage.stagePositions[i].isFree = 0;
                performanceStage.stagePositions[i].currPerformer = currPerformer;
                performanceStage.onStageTotal++;
                positionName = performanceStage.stagePositions[i].positionName;
                stageIndex = i;
                sem_post(&stageLock);
                break;
            }
        }
        sem_post(&stageLock);

        printStage();

        if (performerId < 10)
        {
            printf("%s #0%d joined at %s, performing for %d milliseconds\n\n",
                   performerTypeName, performerId, positionName, randPerformDuration);
        }
        else
        {
            printf("%s #%d joined at %s, performing for %d milliseconds\n\n", performerTypeName, performerId, positionName, randPerformDuration);
        }
        // sem_post(&printSem);

        // sem_post(&enterSem);

        usleep(randPerformDuration * 100);

        // sem_wait(&leaveStageSem);

        performanceStage.stagePositions[stageIndex].isFree = 1;
        performanceStage.stagePositions[stageIndex].currPerformer = NULL;
        performanceStage.onStageTotal--;

        // sem_wait(&printSem);

        // if (performerId < 10)
        // {

        //     printf("DONE:  %s #0%d is done performing at %s \n\n",
        //            performerTypeName, performerId, positionName);
        // }
        // else
        // {
        //     printf("DONE:  %s #%d is done performing at %s \n\n",
        //            performerTypeName, performerId, positionName);
        // }
        sem_post(&stageSem);
        sem_wait(lock);
        (*totalPerformers)--;
        if (!*totalPerformers)
        {
            sem_post(&emptySem);
        }
        sem_post(lock);
    }
    else
    {
        sem_wait(&emptySem);
        sem_wait(&stageSem);

        sem_wait(&stageLock);
        for (int i = 0; i < 4; i++)
        {
            if (performanceStage.stagePositions[i].isFree)
            {
                performanceStage.stagePositions[i].isFree = 0;
                performanceStage.stagePositions[i].currPerformer = currPerformer;
                performanceStage.onStageTotal++;
                positionName = performanceStage.stagePositions[i].positionName;
                stageIndex = i;
                sem_post(&stageLock);
                break;
            }
        }
        sem_post(&stageLock);

        printStage();

        if (performerId < 10)
        {
            printf("%s #0%d joined at %s, performing for %d milliseconds\n\n",
                   performerTypeName, performerId, positionName, randPerformDuration);
        }
        else
        {
            printf("%s #%d joined at %s, performing for %d milliseconds\n\n", performerTypeName, performerId, positionName, randPerformDuration);
        }
        // sem_post(&printSem);

        // sem_post(&enterSem);

        usleep(randPerformDuration * 10);

        // sem_wait(&leaveStageSem);

        performanceStage.stagePositions[stageIndex].isFree = 1;
        performanceStage.stagePositions[stageIndex].currPerformer = NULL;
        performanceStage.onStageTotal--;

        // sem_wait(&printSem);

        // if (performerId < 10)
        // {

        //     printf("DONE:  %s #0%d is done performing at %s \n\n",
        //            performerTypeName, performerId, positionName);
        // }
        // else
        // {
        //     printf("DONE:  %s #%d is done performing at %s \n\n",
        //            performerTypeName, performerId, positionName);
        // }
        sem_post(&stageSem);
        sem_post(&emptySem);
    }

    // sem_post(&printSem);

    // if (isCorrect = 1)
    //     sem_post(&napSem);

    // sem_post(&leaveStageSem);

    // sem_post(&stageSem);
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
    sem_init(&napSem, 0, 0);

    sem_init(&stageLock, 0, 1);
    sem_init(&fdancerLock, 0, 1);
    sem_init(&jugglerLock, 0, 1);
    sem_init(&emptySem, 0, 1);
    sem_init(&orderSem, 0, 1);

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
    createThreads(soloistThreads, SOLOISTS_TOT, soloists);
    createThreads(flamencoThreads, FLAMENCO_TOT, flamencos);

    joinThreads(jugglerThreads, JUGGLER_TOT);
    joinThreads(soloistThreads, SOLOISTS_TOT);
    joinThreads(flamencoThreads, FLAMENCO_TOT);

    printf("---------------------------------------------------------------\n");
    printf("woohoo its over yay\n");
    printf("---------------------------------------------------------------\n");

    return 0;
}