#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#define MAX_CHAR 255

#define JUGGLER_TOT 8
#define FLAMENCO_TOT 15
#define SOLOISTS_TOT 2

#define JUGGLER 0
#define FLAMENCO 1
#define SOLOIST 2

#define STAGE_POSITIONS 4

struct performer
{
    char *typeName;
    int id;
    int performType;
} performer;

struct stagePosition
{
    char *positionName;
    int isFree;
    struct performer *currPerformer;
} stagePosition;

struct stage
{
    int onStageTotal;
    struct stagePosition stagePositions[4];
} stage;

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
            printf("Seed file is empty! Exiting...\n"); //hjhh
            exit(1);
        }
    }
    else
    {
        printf("Cannot read from seed file! Exiting...\n");
        exit(1);
    }
}
