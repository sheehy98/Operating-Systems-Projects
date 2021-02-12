#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h> // importing string.h... are we allowed to use this?
// omfg pointers r confusing lomaoafjosdj
#define MAX_LINES 512
#define FIFO 1
#define SJF 2
#define RR 3

struct job // job linked list struct
{
    int id;
    int length;
    struct job *next; // singly linked list :(
};

// function prototypes
int openFile(struct job **head, char testFile[]);
int checkValidInput(int count, char policy[], int timeSlice);
int getPolicy(char policy[]);
void appendJob(struct job **head, int id, int length);
void removeJob(struct job **head, int key);
void swapSJF(struct job *a, struct job *b);
void bubbleSortSJF(struct job **head);
void printJobs(struct job *head);

int checkValidInput(int count, char policy[], int timeSlice)
{
    int argCountValid = (count <= 4 && count >= 3) ? 1 : 0;
    int policyValid = (strcmp(policy, "FIFO") == 0 || strcmp(policy, "SJF") == 0 || strcmp(policy, "RR") == 0) ? 1 : 0;
    int timeSliceValid = (timeSlice >= 0 && timeSlice <= 100) ? 1 : 0;

    if ((argCountValid && policyValid && timeSliceValid) == 1)
    {
        return 1;
    }
    return 0;
}

int getPolicy(char policy[])
{
    if (strcmp(policy, "FIFO") == 0)
    {
        return FIFO;
    }
    else if (strcmp(policy, "SJF") == 0)
    {
        return SJF;
    }
    else if (strcmp(policy, "RR") == 0)
    {
        return RR;
    }
    else
        return 0;
}

int openFile(struct job **head, char testFile[])
{
    /* row, column, not column row omg */
    char fileLines[MAX_LINES][5]; // setting a max of 512 lines from input file, don't think it'll exceed
    // char **fileLines = malloc(512);
    int idCounter = 0; // use this value to iterate backwards later on
    FILE *fp = fopen(testFile, "r");
    int curLine = 0;
    // Storing data FIFO with append job function
    if (fp != NULL)
    {
        while ((fgets(fileLines[idCounter], MAX_LINES, fp) != NULL))
        {
            appendJob(head, idCounter, atoi(fileLines[idCounter]));
            idCounter++;
        }
        fclose(fp);
        return 1; // success
    }
    return 0; // error
}

void printJobs(struct job *head)
{
    struct job *ptr = (struct job *)malloc(sizeof(struct job));
    while (ptr != NULL)
    {
        // printf("ID: %i, Length: %i\n", ptr->id, ptr->length);
        ptr = ptr->next;
    }
}

void printJob(struct job **head)
{
}

void appendJob(struct job **head, int id, int length)
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

// void removeJob(struct job **head, int key)
// {
//     // store curr head node
//     struct job *temp = *head, *prev;

//     // if head has key (id) to delete
//     if (temp != NULL && temp->id == key)
//     {
//         *head = temp->next; // head changed
//         free(temp);         //free the olde ahh head
//         return;
//     }

//     // else search for key to be added
//     while (temp != NULL && temp->id != key)
//     {
//         prev = temp;
//         temp = temp->next;
//     }

//     // if key not in linked list
//     if (temp == NULL)
//     {
//         printf("Could not find ID to delete :(");
//         return;
//     }

//     // else unlink
//     prev->next = temp->next;

//     free(temp);
//     return;
// }

/* Swap helper function for bubble sort algo */
void swapSJF(struct job *a, struct job *b)
{
    // printf("Swapping ID: %i with ID: %i \n", a->id, b->id);
    int tempId = a->id;
    int tempLength = a->length;
    a->length = b->length;
    a->id = b->id;
    b->length = tempLength;
    b->id = tempId;
    return;
}

/* Generic bubble sort implementation */
void bubbleSortSJF(struct job **head)
{
    int swapped, i;
    struct job *ptrL;
    struct job *ptrR;

    if (head == NULL)
    {
        printf("FFFFF\n");
        return;
    }

    do
    {
        swapped = 0;
        ptrL = *head;
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