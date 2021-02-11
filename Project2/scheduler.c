/* Phase 1 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h> // importing string.h... are we allowed to use this?

#define MAX_LINES 512

struct job // job linkedlist node
{
    int id;
    int length;
    // other meta data to be added?
    struct job *next;
};

int main(int argc, char **argv)
{
    struct job *head = NULL;
    struct job *current = NULL;
    char fileLines[5][MAX_LINES]; // setting a max of 512 lines from input file, don't think it'll exceed
    char *policy = argv[1];
    char *testFile = argv[2];
    char *timeSlice = argv[3];
    int timeSliceInt;
    if (timeSlice != NULL)
    {
        timeSliceInt = atoi(timeSlice); // THIS IS CAUSING A SEG FAULT AHHHH
    }

    void removeJob(struct job * *head, int key)
    {
        // store curr head node
        struct job *temp = *head, *prev;

        // if head has key (id) to delete
        if (temp != NULL && temp->id == key)
        {
            *head = temp->next; // head changed
            free(temp);         //free the olde ahh head
            return;
        }

        // else search for key to be added
        while (temp != NULL && temp->id != key)
        {
            prev = temp;
            temp = temp->next;
        }

        // if key not in linked list
        if (temp == NULL)
        {
            printf("Could not find ID to delete :(");
            return;
        }

        // else unlink
        prev->next = temp->next;

        free(temp);
    }

    /* Insert job at end of linked list as opposed to at start (first in) */
    void appendJob(struct job * *head, int id, int length)
    {
        /* make new node */
        struct job *link = (struct job *)malloc(sizeof(struct job));

        struct job *last = *head;

        /* add data */
        link->id = id;
        link->length = length;

        /* will be last node */
        link->next = NULL;

        /* if list empt, new node is head */
        if (*head == NULL)
        {
            *head = link;
            return;
        }

        /* otherwise go till last node */
        while (last->next != NULL)
            last = last->next;

        /* change last node next */
        last->next = link;
        return;
    }

    void openFile()
    {
        int idCounter = 0; // use this value to iterate backwards later on
        FILE *fp = fopen(testFile, "r");
        int curLine = 0;

        // Storing data FIFO with append job function
        if (fp != NULL)
        {

            while ((fgets(fileLines[idCounter], MAX_LINES, fp) != NULL))
            {
                appendJob(&head, idCounter, atoi(fileLines[idCounter]));
                idCounter++;
            }
        }
        else
        {
            printf("Could not find specified file. Check your argument and try again.\n");
            exit(1);
        }
    }

    void swapSJF(struct job * a, struct job * b)
    {
        int tempId = a->id;
        int tempLength = a->length;
        a->length = b->length;
        a->id = b->id;
        b->length = tempLength;
        b->id = tempId;
    }

    void bubbleSortSJF(struct job * head)
    {
        int swapped, i;
        struct job *ptrL;
        struct job *ptrR;

        if (head == NULL)
            return;

        do
        {
            swapped = 0;
            ptrL = head;

            while (ptrL->next != ptrR)
            {
                if (ptrL->length > ptrL->next->length)
                {
                    swapSJF(ptrL, ptrL->next);
                    swapped = 1;
                }
                ptrL = ptrL->next;
            }
            ptrR = ptrL;
        } while (swapped);
    }

    // if not providing between 3 and 4 arguments, scream!
    if (!(argc >= 3 && argc <= 4))
    {
        printf("Requires at least 2 arguments: Policy, tests/x.in.\n");
        return 0;
    }

    // time slice is invalid, should be between 0 and 100 realistically
    if (!(timeSliceInt >= 0 && timeSliceInt < 100))
    {
        printf("Time slice value invalid. Choose between 0 and 100.\n");
        return 0;
    }

    /* Open le file, fill le array */
    openFile();

    // switch case with if else, not working with ints could use enum to create a struct
    if (strcmp(policy, "FIFO") == 0)
    {
        printf("Execution trace with %s:\n", policy);
        int idCounter = 0; // use this value to iterate backwards later on
        struct job *ptr = head;
        while (ptr != NULL)
        {
            printf("Job %d ran for: %d\n", ptr->id, ptr->length);
            ptr = ptr->next;
            idCounter--;
        }
        printf("End of execution with %s.\n", policy);
    }

    else if (strcmp(policy, "SJF") == 0)
    {
        printf("Execution trace with %s:\n", policy);
        bubbleSortSJF(head);
        struct job *ptr = head;
        while (ptr != NULL)
        {
            printf("Job %d ran for: %d\n", ptr->id, ptr->length);
            ptr = ptr->next;
        }
        printf("End of execution with %s.\n", policy);
    }

    else if (strcmp(policy, "RR") == 0)
    {
        // KEEP TRACK OF:
        // length of time slice provided
        // length of job
        // HOW TO DO:
        // if (time > 0) {
        // if time > than time slice
        // print out job ran for timeslice
        // subtract joblength - timeslice
        // add back to queue
        // else if time <= time slice
        // }
        // run job for allotted amount of time
        // somehow remove

        if (argc != 4)
        {
            printf("You must provide a time slice value for RR\n");
            return 0;
        }
        // struct job *ptr = head;
        printf("Execution trace with %s:\n", policy);
        struct job *ptr = head;
        while (1)
        {
            // int isFinished = 1;
            int newLength = 0;

            if (ptr->length > 0)
            {
                // isFinished = 0; // pending process

                // if job length is greater than time slice
                if (ptr->length > timeSliceInt)
                {
                    printf("Job %i ran for: %i\n", ptr->id, timeSliceInt);
                    newLength = ptr->length - timeSliceInt;
                    // removeJob(&ptr, ptr->id);
                    appendJob(&ptr, ptr->id, newLength);
                }
                else
                {
                    printf("Job %i ran for: %i\n", ptr->id, ptr->length);
                    // removeJob(&ptr, ptr->id);
                }
                if ((ptr = ptr->next) == NULL)
                    break;
            }

            // if (isFinished == 1)
            // {
            //     printf("BREAK\n");
            //     break;
            // }
        }
        printf("End of execution with %s.\n", policy);

        // while (ptr != NULL)
        // {
        //     int timeLeft;

        //     if (ptr->length > 0)
        //     {
        //         if (ptr->length > timeSliceInt)
        //         {
        //             printf("Job %i ran for: %i\n", head->id, timeSliceInt);
        //             timeLeft = ptr->length - timeSliceInt;
        //             appendJob(&ptr, ptr->id, timeLeft);
        //             ptr = ptr->next;
        //         }
        //         // else
        //         {
        //             printf("Job %i ran for: %i\n", head->id, ptr->length);
        //             removeJob(&ptr, ptr->id);
        //             ptr = ptr->next;
        //         }
        //     }
        // }
    }
    else
    {
        printf("Enter FIFO SJF or RR as the policy\n");
    }

    return 0;
    // printf("Hello, please help me schedule!");
}