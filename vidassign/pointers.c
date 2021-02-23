#include <stdio.h>
#include <stdlib.h>
struct student
{
    int age;
    float weight;
    char name[30];
};

int main()
{
    struct student *ptr;
    int i, n;

    printf("Enter the number of students: ");
    scanf("%d", &n);

    // allocating memory for n numbers of struct student
    ptr = (struct student *)malloc(n * sizeof(struct student));

    for (i = 0; i < n; ++i)
    {
        printf("Enter name and age: ");
        scanf("%s %d", (ptr)->name, &(ptr)->age);
        ptr++;
    }

    printf("Accessing with pointers:\n");
    for (i = 0; i < n; ++i)
    {
        printf("Name: %s\tAge: %d\n", (ptr)->name, (ptr)->age);
        ptr++;
    }

    return 0;
}