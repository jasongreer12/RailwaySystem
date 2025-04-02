/*
Author: Jarett Woodard
Group: B
*/
#ifndef PARSER_H
#define PARSER_H

#define ITEM_CHAR_MAX 64
#define ITEM_COUNT_MAX 64
#define LINE_MAX 256

/* This is the header file for the parser module.
*/

/* Struct to hold one train ID and its route (sequence of intersection names)
Ex. Train1:IntersectionA,IntersectionB,IntersectionC
This parses to
id = "Train1"
route[0] = "IntersectionA"
route[1] = "IntersectionB"
route[2] = "IntersectionC"
...
route[64] = "Intersection_N"

*/
typedef struct {
    char id[ITEM_CHAR_MAX];                             // Train name (e.g., "Train1")
    char route[ITEM_COUNT_MAX][ITEM_CHAR_MAX];          // Ordered intersection list
    int routeLength;                                    // Number of intersections
} TrainEntry;

/* Struct to hold one intersection's ID, capacity, and runtime available spots
ex. IntersectionA:1
This parses to
id = "IntersectionA"
capacity = 1
available = 1
...
initial condition is that available = capacity. Capcity can be reduced as trains
occupy the intersection. An initial capcity of 1 indicates a mutex. An initial
capacity of >1 indicates a semaphore.
*/
typedef struct {
    char id[ITEM_CHAR_MAX];
    int capacity;
    int available;
} IntersectionEntry;

// functions to call from main
int getTrains(TrainEntry trains[]);
int getIntersections(IntersectionEntry intersections[]);

// Optional debug print functions
void printTrainEntries(const TrainEntry trains[], int count);
void printIntersectionEntries(const IntersectionEntry intersections[], int count);

#endif  // PARSER_H
