/* Phase 1 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h> // importing string.h... are we allowed to use this?

#define MAX_LINES 512

struct job // node for linked list
{
    int id;
    int length;
    // other meta data to be added?
    struct job *next;
};

struct job *head = NULL;
struct job *current = NULL;

void insertJob(int id, int length)
{
    // create link
    struct job *link = (struct job *)malloc(sizeof(struct job));

    // insert data
    link->id = id;
    link->length = length;

    // point to previous node
    link->next = head;

    // point to new node
    head = link;
}

void printJobs()
{
    struct job *ptr = head;
    // printf("\n[head]=>");

    while (ptr != NULL)
    {
        printf("ID: %d, Length: %d\n", ptr->id, ptr->length);
        ptr = ptr->next;
    }
    printf("NULL\n");
}

int main(int argc, char **argv)
{
    int idCounter = 0;            // use this value to iterate backwards later on
    char fileLines[5][MAX_LINES]; // setting a max of 512 lines from input file, don't think it'll exceed
    char directoryFile[15] = "tests/";
    char *policy = argv[1];
    char *testFile = argv[2];
    char *timeSlice = argv[3];
    int timeSliceInt = atoi(timeSlice);

    // if not providing between 3 and 4 arguments, scream!
    if (!(argc >= 3 && argc <= 4))
    {
        printf("Please fill out the appropriate amount of arguments!\n");
        return 0;
    }

    // time slice is invalid, should be between 0 and 100 realistically
    if (!(timeSliceInt > 0 && timeSliceInt < 100))
    {
        printf("Time slice value invalid. Choose between 0 and 100.\n");
        return 0;
    }

    sprintf(directoryFile + strlen(directoryFile), "%s", testFile);
    FILE *fp = fopen(directoryFile, "r");
    int curLine = 0;

    // I'm storing data in reverse here, keep that in mind!
    if (fp != NULL)
    {

        while ((fgets(fileLines[idCounter], MAX_LINES, fp) != NULL))
        {
            // printf("%d\n", fileLines[idCounter]);
            insertJob(idCounter, atoi(fileLines[idCounter]));
            idCounter++;
        }
        // else
        // {
        //     printf("file was empty");
        // }
    }
    else
    {
        printf("Could not find specified file. Check your argument and try again.\n");
        return (0);
    }

    printJobs();

    // switch case with if else, not working with ints
    if (strcmp(policy, "FIFO") == 0)
    {
        printf("FIFO\n");
    }

    else if (strcmp(policy, "STCF") == 0)
    {
        printf("STCF\n");
    }

    else if (strcmp(policy, "RR") == 0)
    {
        if (argc != 4)
        {
            printf("You must provide a time slice value for RR\n");
            return 0;
        }
        printf("RR\n");
    }

    else
    {
        printf("Enter FIFO STCF or RR as the policy\n");
    }

    return 0;
    // printf("Hello, please help me schedule!");
}