#include <stdio.h>
#include <stdlib.h>
#define MAX_CHAR 255

int main()
{
    FILE *fp = fopen("seed.txt", "r"); //file pointer, set to read only
    char seedBuffer[MAX_CHAR];         //char array to store seed from seed.text
    int randSeed;                      //the seed to be converted to int

    //check if can't read file
    //MOVE THIS INTO HELPERS.C
    if (fp != NULL)
    {
        //I'm assuming there will only be one line here
        //for future implementations I'll learn how to only read
        //from the first line if only first line is necessary
        if (fgets(seedBuffer, MAX_CHAR, fp) != NULL)
        {
            randSeed = atoi(seedBuffer);
            fclose(fp);
            return randSeed;
        }
        else
        {
            printf("Seed file is empty! Exiting...\n");
            fclose(fp);
            exit(1); //EXIT FAIL
        }
    }
    else
    {
        printf("Cannot read from seed file! Exiting...\n");
        exit(1); //EXIT FAIL
    }
    return 0;
}