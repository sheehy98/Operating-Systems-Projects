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

struct completedJob
{
    int id;
    int length;
    int rTime;
    int tTime;
    struct completedJob *next;
};

// function prototypes
int openFile(struct job **head, char testFile[]);
int checkValidInput(int count, char policy[], int timeSlice);
int getPolicy(char policy[]);
void appendJob(struct job **head, int id, int length);
void appendCompletedJob(struct completedJob **head, int id,
                        int rTime, int tTime, int length);
void removeJob(struct job **head, int key);
void swapSJF(struct job *a, struct job *b);
void bubbleSortSJF(struct job **head);
void printJobs(struct job **head, char policy[]);
int cjExists(struct completedJob **head, int id);
void editCJNode(struct completedJob **head, int id, int tTime);

void responseTimeRR(struct job **head)
{
    struct job *ptr = *head;
    while (ptr->next != NULL)
    {
    }
}

void editCJNode(struct completedJob **head, int id, int tTime)
{
    struct completedJob *ptr = *head;
    while (ptr->id != id)
    {
        ptr = ptr->next;
    }

    ptr->tTime = tTime;

    return;
}

int cjExists(struct completedJob **head, int id)
{
    struct completedJob *ptrCJ = *head;
    while (ptrCJ != NULL)
    {
        if (ptrCJ->id == id)
        {
            return 1;
        }
        else
        {
            ptrCJ = ptrCJ->next;
        }
    }
    return 0;
}

void policyAnalysis(struct job **head, char policy[])
{
    struct job *ptr = *head;
    int response = 0;
    int totResponse = 0;
    int turnaround = 0;
    int totTurnaround = 0;
    float numJobs = 0;
    float responseAvg = 0;
    float turnaroundAvg = 0;

    printf("Begin analyzing %s:\n", policy);
    while (ptr != NULL)
    {
        turnaround += ptr->length;
        totTurnaround += turnaround;
        printf("Job %i -- Response time: %i  Turnaround: %i  Wait: %i\n",
               ptr->id, response, turnaround, response);
        totResponse += response;
        response += ptr->length;
        ptr = ptr->next;
        numJobs++;
    }
    responseAvg = (float)totResponse / numJobs;
    turnaroundAvg = (float)totTurnaround / numJobs;
    printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n",
           responseAvg, turnaroundAvg, responseAvg);
    printf("End analyzing %s.\n", policy);
}

void rrPlusPolicyAnalysis(int timeSliceInt, char policy[], struct job **head, struct completedJob **headCJ)
{

    printf("Execution trace with %s:\n", policy);
    struct job *ptr = *head;

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
                printf("Job %i ran for: %i\n", ptr->id, timeSliceInt);
                newLength = ptr->length - timeSliceInt;
                tTime += timeSliceInt;
                appendJob(&ptr, ptr->id, newLength);
                if (cjExists(headCJ, ptr->id) == 0)
                {
                    appendCompletedJob(headCJ, ptr->id, rTime,
                                       tTime, ptr->length);
                }
                else
                {
                    struct completedJob *ptrCJ = *headCJ;
                    editCJNode(headCJ, ptr->id, tTime);
                }
                rTime += timeSliceInt;
            }
            else
            {
                tTime += ptr->length;
                printf("Job %i ran for: %i\n", ptr->id, ptr->length);
                if (cjExists(headCJ, ptr->id) == 0)
                {
                    appendCompletedJob(headCJ, ptr->id, rTime, tTime, ptr->length);
                }
                else
                {
                    struct completedJob *ptrCJ = *headCJ;
                    editCJNode(headCJ, ptr->id, tTime);
                }
                rTime += ptr->length;
            }
            if ((ptr = ptr->next) == NULL)
                break;
        }
    }
    printf("End of execution with %s.\n", policy);
    printf("Begin analyzing %s:\n", policy);
    struct completedJob *ptrCJ = *headCJ;
    float numJobs = 0;
    int totResponse = 0;
    int totTurnaround = 0;
    int totWait = 0;
    float responseAvg = 0;
    float turnaroundAvg = 0;
    float waitAvg = 0;
    while (ptrCJ != NULL)
    {
        printf("Job %i -- Response time: %i  Turnaround: %i  Wait: %i\n", ptrCJ->id, ptrCJ->rTime, ptrCJ->tTime, (ptrCJ->tTime - ptrCJ->length));
        totResponse += ptrCJ->rTime;
        totTurnaround += ptrCJ->tTime;
        totWait += (ptrCJ->tTime - ptrCJ->length);
        ptrCJ = ptrCJ->next;
        numJobs++;
    }
    responseAvg = (float)totResponse / numJobs;
    turnaroundAvg = (float)totTurnaround / numJobs;
    waitAvg = (float)totWait / numJobs;
    printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n",
           responseAvg, turnaroundAvg, waitAvg);
    printf("End analyzing %s.\n", policy);
}

void printJobs(struct job **head, char policy[])
{
    struct job *ptr = *head;
    printf("Execution trace with %s:\n", policy);
    while (ptr != NULL)
    {
        printf("Job %d ran for: %d\n", ptr->id, ptr->length);
        ptr = ptr->next;
    }
    printf("End of execution with %s.\n", policy);
}

int checkValidInput(int count, char policy[], int timeSlice)
{
    int argCountValid = (count <= 4 && count >= 3) ? 1 : 0;
    int policyValid = (strcmp(policy, "FIFO") == 0 ||
                       strcmp(policy, "SJF") == 0 ||
                       strcmp(policy, "RR") == 0)
                          ? 1
                          : 0;
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
    int idCounter = 0;            // use this value to iterate backwards later on
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

void appendCompletedJob(struct completedJob **head, int id,
                        int rTime, int tTime, int length)
{

    /* make new node */
    struct completedJob *link = (struct completedJob *)
        malloc(sizeof(struct completedJob));
    struct completedJob *last = *head;

    /* add data */
    link->id = id;
    link->rTime = rTime;
    link->tTime = tTime;
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

void removeJob(struct job **head, int key)
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
    return;
}

/* Swap helper function for bubble sort algo */
void swapSJF(struct job *a, struct job *b)
{
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
    struct job *ptrL = NULL;
    struct job *ptrR = NULL;

    if (head == NULL)
    {
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