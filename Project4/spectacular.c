#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include "spectacular.h"

/* semaphores used in program, some are used as pseudo-locks and are named as such */
sem_t stageSem, printSem, stageLock, fdancerLock, jugglerLock, emptySem, orderSem;

/* array of structs for each of the performer types */
struct performer jugglers[JUGGLER_TOT] = {NULL};
struct performer flamencos[FLAMENCO_TOT] = {NULL};
struct performer soloists[SOLOISTS_TOT] = {NULL};

/* the stage that the performers are performing on, has four positions */
struct stage performanceStage;

/* total number of dancers or jugglers on stage at a given moment */
int totalfDancers, totalJugglers;

/* creates the stage with each of the positions on stage, sets free flag to 1 */
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

/* prints which performers are currently on stage - bonus points plz */
void printStage()
{
    printf("[ ");
    for (int i = 0; i < 4; i++)
    {
        if (performanceStage.stagePositions[i].currPerformer != NULL)
        {
            printf(performanceStage.stagePositions[i].currPerformer->id < 10 ? "%s #0%d " : "%s #%d ",
                   performanceStage.stagePositions[i].currPerformer->typeName,
                   performanceStage.stagePositions[i].currPerformer->id);
        }
        else
        {
            printf("________ ");
        }
    }
    printf("] ");
}

/* creates the performers */
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

/* helper function for performerFunction */
void perform(struct performer *currPerformer)
{
    char *positionName;                                                 // stores the name of the current position on stage (Position Zero etc.)
    int stageIndex;                                                     // numerical representation of the position on stage
    int performerId = currPerformer->id;                                // the performer's id
    char *performerTypeName = currPerformer->typeName;                  // the performer's type as a string ("Juggler" etc.)
    int randPerformDuration = rand() % (1000000 - 100000 + 1) + 100000; // the duration of the performance, between .1 and .01 seconds

    sem_wait(&stageSem);  // only allows four performers on stage at a time
    sem_wait(&stageLock); // stage lock makes sure each performer joining the stage is atomic
    /* puts performer in the first available stage position. because we
       use stageSem, there will always be one available stage position*/
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
    sem_post(&stageLock); // allows next performer to get on stage

    /* after the performer joins the stage, it prints its position on the stage and performs for a random duration */
    sem_wait(&printSem); // prevents prints from being interrupted
    printf("JOIN: ");
    printStage();
    printf("| ");
    printf(performerId < 10 ? "%s #0%d joined at %s, performing for %d microseconds\n\n" : "%s #%d joined at %s, performing for %d microseconds\n\n",
           performerTypeName, performerId, positionName, randPerformDuration);
    sem_post(&printSem);

    usleep(randPerformDuration); // performer performing for a random duration

    /* below lines simulate performer leaving the stage */
    performanceStage.stagePositions[stageIndex].isFree = 1;
    performanceStage.stagePositions[stageIndex].currPerformer = NULL;
    performanceStage.onStageTotal--;

    sem_wait(&printSem); // prevents prints from being interrupted
    printf("DONE: ");
    printStage();
    printf("| ");
    printf(performerId < 10 ? "%s #0%d is done performing at %s \n\n" : "%s #%d is done performing at %s \n\n",
           performerTypeName, performerId, positionName);
    sem_post(&printSem);

    sem_post(&stageSem);
}

void *performerFunction(void *arg)
{
    struct performer *currPerformer = ((struct performer *)arg); // cast arg to performer struct
    int performerTypeInt = currPerformer->performType;           // the performer's type in integer (0 = juggler, etc)

    sem_t *lock = performerTypeInt == 0 ? &jugglerLock : &fdancerLock; // sets pointer to pseudo-lock semaphore corresponding
                                                                       // to the given performer type

    // below is the jugglers and dancers case, they have identical logic (thus within the same if scope)
    if (performerTypeInt < 2)
    {
        sem_wait(&orderSem);                                                            // applies an order on the performers attempting to enter so as no to cause starvation of other performers
        sem_wait(lock);                                                                 // "locks" the total performers (either total dancers or jugglers) to keep the iteration atomic
        int *totalPerformers = performerTypeInt == 0 ? &totalfDancers : &totalJugglers; // pointer to either total dancers or jugglers
        (*totalPerformers)++;                                                           // iterate performers
        if (*totalPerformers == 1)                                                      // if at any point there is one performer on stage of a certain type, set emptySem to wait
        {
            sem_wait(&emptySem);
        }
        sem_post(&orderSem); // order has been kept, no body is starving
        sem_post(lock);      // atomic instruction is over

        perform(currPerformer); // the performer can now perform

        sem_wait(lock); // lock the atomic instruction of decrementing the total performer count
        (*totalPerformers)--;
        if (!*totalPerformers) // if at any point the number of total performers of a certain type = 0, post emptySem to give the soloists a chance to perform
        {
            sem_post(&emptySem);
        }
        sem_post(lock); // atomic instruction is over
    }
    // below is the soloist case, it only checks if the stage is empty before performing
    else
    {
        sem_wait(&emptySem);

        perform(currPerformer);

        sem_post(&emptySem);
    }
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
    int randSeed = getSeed(); // get the seed from seed.txt
    printf("Random seed is: %d\n", randSeed);
    printf("Seeding the randomizer...\n");
    srand(randSeed); // seed the randomizer

    /* initializing semaphores */
    sem_init(&stageSem, 0, 4);
    sem_init(&printSem, 0, 1);
    sem_init(&stageLock, 0, 1);
    sem_init(&fdancerLock, 0, 1);
    sem_init(&jugglerLock, 0, 1);
    sem_init(&emptySem, 0, 1);
    sem_init(&orderSem, 0, 1);

    printf("---------------------------------------------------------------\n");
    printf("Welcome one and all to the summer spectacular. Skrrrrrrrrrrtttt\n");
    printf("---------------------------------------------------------------\n\n");

    printf("Stage positions are as follows:\n");
    printf("JorL: [ __POS0__ __POS1__ __POS2__ __POS3__ ]\n\n");
    printf("---------------------------------------------------------------\n\n");

    sleep(1);

    /* initialize the threads, provide them enough memory (based on total number of each performer type) */
    pthread_t *jugglerThreads = (pthread_t *)malloc(JUGGLER_TOT * sizeof(pthread_t));
    pthread_t *flamencoThreads = (pthread_t *)malloc(FLAMENCO_TOT * sizeof(pthread_t));
    pthread_t *soloistThreads = (pthread_t *)malloc(SOLOISTS_TOT * sizeof(pthread_t));

    createStage();      // init stage
    createPerformers(); // init performers

    /* spawn threads */
    createThreads(jugglerThreads, JUGGLER_TOT, jugglers);
    createThreads(flamencoThreads, FLAMENCO_TOT, flamencos);
    createThreads(soloistThreads, SOLOISTS_TOT, soloists);

    /* join threads */
    joinThreads(jugglerThreads, JUGGLER_TOT);
    joinThreads(flamencoThreads, FLAMENCO_TOT);
    joinThreads(soloistThreads, SOLOISTS_TOT);

    printf("---------------------------------------------------------------\n");
    printf("woohoo its over yay\n");
    printf("---------------------------------------------------------------\n");

    /* free memory */
    free(jugglerThreads);
    free(flamencoThreads);
    free(soloistThreads);

    /* finis */
    return 0;
}
