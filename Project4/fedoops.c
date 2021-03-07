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
        printf("Package #%d with no. instructs %d \n", ptr->packageNum, ptr->totInstructions);
        printf("Instructions array = \n");
        printf("[ ");
        for (int i = 0; i < ptr->totInstructions; i++)
        {
            printf("\t%d", ptr->custInstructions[i]);
        }
        printf(" ]\n");
        ptr = ptr->nextPackage;
    }
}

void assignPackages(int packageCount, struct package **head)
{
    struct package *link = (struct package *)malloc(sizeof(struct package));
    struct package *last = *head;
    // struct package *temp = NULL;

    int totInstructions = 0;

    // add data block
    for (int i = 0; i < packageCount; i++)
    {
        totInstructions = (rand() % 4) + 1;
        printf("total number of instructs: %d\n", totInstructions);

        link->packageNum = i + 1;
        link->totInstructions = totInstructions;
        link->ready = 1;
        link->currStation = 0;

        while (j < totInstructions)
        {
            int flag = 0; // flag for duplicate value
            int instruction = (rand() % 4) + 1;

            for (int k = 0; k < j; k++)
            {
                if (instruction == link->custInstructions[k])
                {
                    flag = 1;
                    break;
                }
            }

            if (flag != 1)
            {
                link->custInstructions[j] = instruction;
                j++;
            }
        }

        // link->nextPackage = NULL;

        if (*head == NULL)
        {
            *head = link;
            // return;
        }

        // link = link->nextPackage;

        // while (last->nextPackage != NULL)
        //     last = last->nextPackage;

        // last->nextPackage = link;
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