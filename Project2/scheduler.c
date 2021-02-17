/* Phase 1 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h> // importing string.h... are we allowed to use this?
#include "scheduler_help.h"

int main(int argc, char **argv)
{

    struct job *head = NULL;
    struct completedJob *headCJ = NULL;
    char *policy = argv[1];
    char *testFile = argv[2];
    char *timeSlice = argv[3];
    int timeSliceInt = timeSlice != NULL ? atoi(timeSlice) : 0;

    if (checkValidInput(argc, policy, timeSliceInt) == 1)
    {
        if (openFile(&head, testFile) != 0)
        {
            switch (getPolicy(policy))
            {
            case (FIFO):
            {
                printJobs(&head, policy);
                policyAnalysis(&head, policy);
                break;
            }
            case (SJF):
            {
                bubbleSortSJF(&head);
                printJobs(&head, policy);
                policyAnalysis(&head, policy);
                break;
            }
            case (RR):
            {
                if (timeSliceInt == 0)
                {
                    printf("You're going to break your computer. Try again.\n");
                    return 0;
                }

                rrPlusPolicyAnalysis(timeSliceInt, policy, &head, &headCJ);
                break;
            }
            default:
                printf("Didn't work\n");
                break;
            }
        }
        else
        {
            printf("Could not find specified file. Check your arguments and try again.\n");
            return 0;
        }
    }
    else
    {
        printf("Check input and try again.\n");
        return 0;
    }

    return 0;
}