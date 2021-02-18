/* Phase 1 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "scheduler_help.h"

int main(int argc, char **argv)
{

    struct job *head = NULL;                                    // job linked list head
    struct completedJob *headCJ = NULL;                         // completed job (linked list for RR analysis)
    char *policy = argv[1];                                     // policy argument
    char *testFile = argv[2];                                   // test file+path argument
    char *timeSlice = argv[3];                                  // time slice argument
    int timeSliceInt = timeSlice != NULL ? atoi(timeSlice) : 0; // if no timeslice passed, assume 0 (prevent segfault)

    if (checkValidInput(argc, policy, timeSliceInt) == 1) // check if arguments are correct
    {
        if (openFile(&head, testFile) != 0) // check if file opened as expected
        {
            switch (getPolicy(policy)) // switch based on policy argument
            {
            case (FIFO): // FIFO case
            {
                printJobs(&head, policy);
                policyAnalysis(&head, policy);
                break;
            }
            case (SJF): // SJF case
            {
                bubbleSortSJF(&head);
                printJobs(&head, policy);
                policyAnalysis(&head, policy);
                break;
            }
            case (RR): // RR case
            {
                if (timeSliceInt == 0) // if user passes in RR with timeslice of 0, going to run infinitely
                {
                    printf("You're going to break your computer. Try again.\n"); // don't want that
                    return 0;
                }

                rrPlusPolicyAnalysis(timeSliceInt, policy, &head, &headCJ);
                break;
            }
            default:
                printf("Invalid policy!\n");
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
        return 0; // exit program if incorrect input
    }

    return 0;
}