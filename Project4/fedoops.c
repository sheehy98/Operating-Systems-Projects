#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include "fedoops.h"

/*
Create simulator for FedOops:
    1) create pile (queue) of pending packages (PPP)
        - each has random set of instructions associated
    2) when worker grabs package, read and follow associated instructions in order
        - 1 to four actions, no action being repeated
    3) syncrhonize workers (each is a thread), stations and packages
    ** print winning team at end (whoever processes most packages)

*/

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

void assignPackages(int packageCount, struct package **head)
{

    int instructionCount = 0;

    for (int i = 0; i < packageCount; i++)
    {
        instructionCount = (rand() % 4) + 1;
        appendPackage(head, i + 1, instructionCount);
    }

    return;
}

int main()
{
    struct package *pileHead = NULL; // pile of pending packages
    int packageCount = 0;
    int randSeed = getSeed();
    printf("Random seed is: %d\n", randSeed);

    srand(randSeed); // seed the randomizer

    // number of packageCount, randomly generated
    // packageCount = rand() % (UPPER - LOWER + 1) + LOWER;
    packageCount = 5;
    printf("Total number of packages to be processed is: %d\n", packageCount);

    assignPackages(packageCount, &pileHead);

    printPackages(&pileHead);

    return 0;
}