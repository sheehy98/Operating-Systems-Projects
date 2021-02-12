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
                // should double check time slice isn't 0, would cause infinite loop
                printf("Execution trace with %s:\n", policy);
                struct job *ptr = head;
                int rTime = 0;
                int tTime = 0;
                while (1)
                {
                    int newLength = 0;
                    if (ptr->length > 0)
                    {
                        // if job length is greater than time slice
                        if (ptr->length > timeSliceInt)
                        {
                            tTime += ptr->length;
                            printf("Job %i ran for: %i\n", ptr->id, timeSliceInt);
                            newLength = ptr->length - timeSliceInt;
                            appendJob(&ptr, ptr->id, newLength);
                            if (!cjExists(&headCJ, ptr->id))
                            {
                                appendCompletedJob(&headCJ, ptr->id, rTime,
                                                   tTime, ptr->length);
                                printf("Job with ID %i, rTime %i, wTime %i, length %i added\n",
                                       ptr->id, rTime, tTime, ptr->length);
                            }
                            else
                            {
                            }
                            rTime += timeSliceInt;
                        }
                        else
                        {
                            tTime += ptr->length;
                            printf("Job %i ran for: %i\n", ptr->id, ptr->length);
                            if (!cjExists(&headCJ, ptr->id))
                            {
                                appendCompletedJob(&headCJ, ptr->id, ptr->length, tTime, ptr->length);
                                printf("Job with ID %i, rTime %i, wTime %i, length %i added\n",
                                       ptr->id, rTime, tTime, ptr->length);
                            }
                            else
                            {
                            }
                            rTime += ptr->length;
                        }
                        if ((ptr = ptr->next) == NULL)
                            break;
                    }
                }
                printf("End of execution with %s.\n", policy);
                // policyAnalysis(&head, policy);
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