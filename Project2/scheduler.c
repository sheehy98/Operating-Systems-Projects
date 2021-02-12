/* Phase 1 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h> // importing string.h... are we allowed to use this?
#include "scheduler_help.h"

// #define MAX_LINES 512
// #define FIFO 1
// #define SJF 2
// #define RR 3

// struct job // job linkedlist node
// {
//     int id;
//     int length;
//     // other meta data to be added?
//     struct job *next; //
// };

int main(int argc, char **argv)
{

    struct job *head = NULL;
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
                // printJobs(head);
                printf("Execution trace with %s:\n", policy);
                struct job *ptr = head;
                if (ptr == NULL)
                    printf("THE HEAD IS NULL WTH!!\n");
                while (ptr != NULL)
                {
                    printf("Job %d ran for: %d\n", ptr->id, ptr->length);
                    ptr = ptr->next;
                }
                printf("End of execution with %s.\n", policy);
                break;
                // case (0):
            }
            case (SJF):
            {
                printf("Execution trace with %s:\n", policy);
                printJobs(head); // adding this line will seemingly run from terminal, but fails in test, what going on
                bubbleSortSJF(&head);
                struct job *ptr = head;
                while (ptr != NULL)
                {
                    printf("Job %d ran for: %d\n", ptr->id, ptr->length);
                    ptr = ptr->next;
                }
                printf("End of execution with %s.\n", policy);
                break;
            }
            case (RR):
            {
                // should double check time slice isn't 0, would cause infinite loop
                printf("Execution trace with %s:\n", policy);
                struct job *ptr = head;
                while (1)
                {
                    int newLength = 0;
                    if (ptr->length > 0)
                    {
                        // if job length is greater than time slice
                        if (ptr->length > timeSliceInt)
                        {
                            printf("Job %i ran for: %i\n", ptr->id, timeSliceInt);
                            newLength = ptr->length - timeSliceInt;

                            appendJob(&ptr, ptr->id, newLength);
                        }
                        else
                        {
                            printf("Job %i ran for: %i\n", ptr->id, ptr->length);
                        }
                        if ((ptr = ptr->next) == NULL)
                            break;
                    }
                }
                printf("End of execution with %s.\n", policy);
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

    // // if not providing between 3 and 4 arguments, scream!
    // if (!(argc >= 3 && argc <= 4))
    // {
    //     printf("Requires at least 2 arguments: Policy, tests/x.in.\n");
    //     return 0;
    // }

    // // time slice is invalid, should be between 0 and 100 realistically
    // if (!(timeSliceInt >= 0 && timeSliceInt < 100))
    // {
    //     printf("Time slice value invalid. Choose between 0 and 100.\n");
    //     return 0;
    // }

    // /* Open le file, fill le array */
    // openFile();

    // // switch case with if else, not working with ints could use enum to create a struct
    // if (strcmp(policy, "FIFO") == 0)
    // {
    //     printf("Execution trace with %s:\n", policy);
    //     int idCounter = 0; // use this value to iterate backwards later on
    //     struct job *ptr = head;
    //     while (ptr != NULL)
    //     {
    //         printf("Job %d ran for: %d\n", ptr->id, ptr->length);
    //         ptr = ptr->next;
    //         idCounter--;
    //     }
    //     printf("End of execution with %s.\n", policy);
    // }

    // else if (strcmp(policy, "SJF") == 0)
    // {
    //     printf("Execution trace with %s:\n", policy);
    //     bubbleSortSJF(head);
    //     struct job *ptr = head;
    //     while (ptr != NULL)
    //     {
    //         printf("Job %d ran for: %d\n", ptr->id, ptr->length);
    //         ptr = ptr->next;
    //     }
    //     printf("End of execution with %s.\n", policy);
    // }

    // else if (strcmp(policy, "RR") == 0)
    // {
    //     // KEEP TRACK OF:
    //     // length of time slice provided
    //     // length of job
    //     // HOW TO DO:
    //     // if (time > 0) {
    //     // if time > than time slice
    //     // print out job ran for timeslice
    //     // subtract joblength - timeslice
    //     // add back to queue
    //     // else if time <= time slice
    //     // }
    //     // run job for allotted amount of time
    //     // somehow remove

    //     if (argc != 4)
    //     {
    //         printf("You must provide a time slice value for RR\n");
    //         return 0;
    //     }
    //     // struct job *ptr = head;
    //     printf("Execution trace with %s:\n", policy);
    //     struct job *ptr = head;
    //     while (1)'
    //     {
    //         // int isFinished = 1;
    //         int newLength = 0;

    //         if (ptr->length > 0)
    //         {
    //             // isFinished = 0; // pending process

    //             // if job length is greater than time slice
    //             if (ptr->length > timeSliceInt)
    //             {
    //                 printf("Job %i ran for: %i\n", ptr->id, timeSliceInt);
    //                 newLength = ptr->length - timeSliceInt;
    //                 // removeJob(&ptr, ptr->id);
    //                 appendJob(&ptr, ptr->id, newLength);
    //             }
    //             else
    //             {
    //                 printf("Job %i ran for: %i\n", ptr->id, ptr->length);
    //                 // removeJob(&ptr, ptr->id);
    //             }
    //             if ((ptr = ptr->next) == NULL)
    //                 break;
    //         }

    //         // if (isFinished == 1)
    //         // {
    //         //     printf("BREAK\n");
    //         //     break;
    //         // }
    //     }
    //     printf("End of execution with %s.\n", policy);

    //     // while (ptr != NULL)
    //     // {
    //     //     int timeLeft;

    //     //     if (ptr->length > 0)
    //     //     {
    //     //         if (ptr->length > timeSliceInt)
    //     //         {
    //     //             printf("Job %i ran for: %i\n", head->id, timeSliceInt);
    //     //             timeLeft = ptr->length - timeSliceInt;
    //     //             appendJob(&ptr, ptr->id, timeLeft);
    //     //             ptr = ptr->next;
    //     //         }
    //     //         // else
    //     //         {
    //     //             printf("Job %i ran for: %i\n", head->id, ptr->length);
    //     //             removeJob(&ptr, ptr->id);
    //     //             ptr = ptr->next;
    //     //         }
    //     //     }
    //     // }
    // }
    // else
    // {
    //     printf("Enter FIFO SJF or RR as the policy\n");
    // }

    return 0;
    // printf("Hello, please help me schedule!");
}