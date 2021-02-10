/* Phase 1 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv)
{
    char *policy = argv[1];
    char *testFile = argv[2];
    char *timeSlice = argv[3];
    int timeSliceInt = atoi(timeSlice);

    if (!(timeSliceInt > 0 && timeSliceInt < 100))
    {
        printf("Time slice value too large!\n");
        return 0;
    }

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
        printf("RR\n");
    }

    else
    {
        printf("Enter FIFO STCF or RR as the policy\n");
    }

    return 0;
    // printf("Hello, please help me schedule!");
}