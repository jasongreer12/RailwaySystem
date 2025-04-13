/*
Author: Jake Richardson
Group: B
Email: jrich26@okstate.edu
Description: This file randomizes the routes for four trains and saves it to the txt file
trains.txt. The program makes sure to not give different trains the same final destination to
avoid routes that are impossible to complete.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NUM_TRAINS 4
#define ROUTE_LENGTH 3
#define NUM_INTERSECTIONS 5

const char *intersections[] = {
    "IntersectionA",
    "IntersectionB",
    "IntersectionC",
    "IntersectionD",
    "IntersectionE"};

// Check if the intersection is already used as an end point
int is_duplicate_endpoint(const char *endpoint, char *used_endpoints[], int count)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(endpoint, used_endpoints[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int main()
{
    FILE *file = fopen("trains.txt", "w");
    if (file == NULL)
    {
        perror("Failed to open file");
        return 1;
    }

    srand(time(NULL));
    char *used_endpoints[NUM_TRAINS] = {0};

    for (int i = 0; i < NUM_TRAINS; i++)
    {
        char *route[ROUTE_LENGTH];
        int used[NUM_INTERSECTIONS] = {0};
        int step = 0;

        // Generate unique intersections for the route
        while (step < ROUTE_LENGTH)
        {
            int idx = rand() % NUM_INTERSECTIONS;
            if (!used[idx])
            {
                route[step] = (char *)intersections[idx];
                used[idx] = 1;
                step++;
            }
        }

        // Ensure the endpoint is unique among trains
        while (is_duplicate_endpoint(route[ROUTE_LENGTH - 1], used_endpoints, i))
        {
            int idx;
            do
            {
                idx = rand() % NUM_INTERSECTIONS;
            } while (used[idx]);

            route[ROUTE_LENGTH - 1] = (char *)intersections[idx];
            used[idx] = 1;
        }

        used_endpoints[i] = route[ROUTE_LENGTH - 1];

        // Write the route to the file
        fprintf(file, "Train%d:", i + 1);
        for (int j = 0; j < ROUTE_LENGTH; j++)
        {
            fprintf(file, "%s", route[j]);
            if (j < ROUTE_LENGTH - 1)
            {
                fprintf(file, ",");
            }
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("Train routes randomized for trains.txt\n");
    return 0;
}
