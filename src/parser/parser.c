#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

#define LINE_MAX 256
#define ITEM_COUNT_MAX 64
#define ITEM_CHAR_MAX 64

/*This modle will generate train and intersection structs for manipulation elsewhere
  in the program. The train struct will contain a list of intersections that the
  train will pass through. The intersection struct will contain a list of trains
  that are currently at the intersection. The intersection struct will also
  contain a capacity and available spots for trains to wait at the intersection.
  The available spots will be decremented when a train arrives and incremented
  when a train leaves.

  Instructions:
  use #include "parser.h" to include this module in your code.
  Call the getTrains() and getIntersections() functions to get the train and intersection

  functions:
  - static int parseTrainsFile(const char *filepath, TrainEntry trains[]) - parses the train file and returns the number of trains
  - int getTrains(TrainEntry trains[]) - returns an array of TrainEntry structs
  - void printTrainEntries(const TrainEntry trains[], int count) - prints the train entries for debugging
  
  - static int parseIntersectionsFile(const char *filepath, IntersectionEntry intersections[]) - parses the intersection file and returns the number of intersections
  - int parseTrainLine(char *line, TrainEntry *train) - parses a line from the train file and returns a TrainEntry struct
  - void printIntersectionEntries(const IntersectionEntry intersections[], int count) - prints the intersection entries for debugging
*/

// TRAIN PARSER

// Each line from trains.txt will parse into an instance of this struct
static int parseTrainsFile(const char *filepath, TrainEntry trains[]) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Error opening train file");
        return -1;
    }

    char line[LINE_MAX];
    int count = 0;

    while (fgets(line, sizeof(line), file) && count < LINE_MAX) {
        line[strcspn(line, "\n")] = '\0';

        char *id = strtok(line, ":");
        char *valueStr = strtok(NULL, ":");

        if (id && valueStr) {
            strncpy(trains[count].id, id, ITEM_CHAR_MAX - 1);
            trains[count].id[ITEM_CHAR_MAX - 1] = '\0';
            trains[count].routeLength = 0;

            char *token = strtok(valueStr, ",");
            while (token && trains[count].routeLength < ITEM_COUNT_MAX) {
                strncpy(trains[count].route[trains[count].routeLength], token, ITEM_CHAR_MAX - 1);
                trains[count].route[trains[count].routeLength][ITEM_CHAR_MAX - 1] = '\0';
                trains[count].routeLength++;
                token = strtok(NULL, ",");
            }

            count++;
        }
    }

    fclose(file);
    return count;
}

// Getter function for train entries. returns an array of TrainEntry structs
int getTrains(TrainEntry trains[]) {
    return parseTrainsFile("text_files/trains.txt", trains);
}
// Function to print the train entries for debugging
void printTrainEntries(const TrainEntry trains[], int count) {
    for (int i = 0; i < count; i++) {
        printf("Train: %s\n", trains[i].id);
        for (int j = 0; j < trains[i].routeLength; j++) {
            printf("  Route %d: %s\n", j + 1, trains[i].route[j]);
        }
    }
    printf("\n");
}

//INTERSECTION PARSER

// Function to print the intersection entries for debugging
static int parseIntersectionsFile(const char *filepath, IntersectionEntry intersections[]) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Error opening intersection file");
        return -1;
    }

    char line[LINE_MAX];
    int count = 0;

    while (fgets(line, sizeof(line), file) && count < LINE_MAX) {
        line[strcspn(line, "\n")] = '\0';

        char *id = strtok(line, ":");
        char *capStr = strtok(NULL, ":");

        if (id && capStr) {
            strncpy(intersections[count].id, id, ITEM_CHAR_MAX - 1);
            intersections[count].id[ITEM_CHAR_MAX - 1] = '\0';

            intersections[count].capacity = atoi(capStr);
            intersections[count].available = intersections[count].capacity;
            count++;
        }
    }

    fclose(file);
    return count;
}

// Getter function for train entries
int getIntersections(IntersectionEntry intersections[]) {
    return parseIntersectionsFile("text_files/intersections.txt", intersections);
}

// Function to print the intersection entries for debugging
void printIntersectionEntries(const IntersectionEntry intersections[], int count) {
    for (int i = 0; i < count; i++) {
        printf("Intersection: %s\n", intersections[i].id);
        printf("  Capacity:  %d\n", intersections[i].capacity);
        printf("  Available: %d\n", intersections[i].available);
    }
    printf("\n");
}