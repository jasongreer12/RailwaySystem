#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "parser/parser.h"

int main(){
    // create array of train structs and count
    TrainEntry trains[LINE_MAX];
    int trainCount = getTrains(trains);

    // create array of intersections tructs and count
    IntersectionEntry intersections[LINE_MAX];
    int intersectionCount = getIntersections(intersections);
    
    // PRINT CHECK. 
    // Print individual train entries
    printf("Train Entries:\n"); 
    printTrainEntries(trains, trainCount);

    // Print all train entries
    printf("Intersection Entries:\n");
    printIntersectionEntries(intersections, intersectionCount);
}