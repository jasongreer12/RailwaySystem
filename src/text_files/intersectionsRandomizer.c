/*
Author: Jake Richardson
Group: B
Email: jrich26@okstate.edu
Description: This file randomizes the size of intersections A-E and saves the results to intersections.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    FILE *file = fopen("intersections.txt", "w");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    srand(time(NULL)); // Seed random number generator

    const char *intersections[] = {
        "IntersectionA",
        "IntersectionB",
        "IntersectionC",
        "IntersectionD",
        "IntersectionE"};

    int numIntersections = sizeof(intersections) / sizeof(intersections[0]);

    for (int i = 0; i < numIntersections; i++)
    {
        int value = rand() % 3 + 1; // Random number between 1 and 3
        fprintf(file, "%s:%d\n", intersections[i], value);
    }

    fclose(file);
    printf("Intersections randomized for intersections.txt\n");

    return 0;
}
