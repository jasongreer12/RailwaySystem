#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "parser.h"

// Test for correct parsing of trains
void test_getTrains() {
    TrainEntry trains[LINE_MAX];
    int count = getTrains(trains);
    assert(count == 4);  // You expect 4 trains in the file

    // Check Train1
    assert(strcmp(trains[0].id, "Train1") == 0);
    assert(trains[0].routeLength == 3);
    assert(strcmp(trains[0].route[0], "IntersectionA") == 0);
    assert(strcmp(trains[0].route[1], "IntersectionB") == 0);
    assert(strcmp(trains[0].route[2], "IntersectionC") == 0);

    // Check Train4
    assert(strcmp(trains[3].id, "Train4") == 0);
    assert(strcmp(trains[3].route[2], "IntersectionD") == 0);

    printf("test_getTrains passed\n");
}

// Test for correct parsing of intersections
void test_getIntersections() {
    IntersectionEntry intersections[LINE_MAX];
    int count = getIntersections(intersections);
    assert(count == 5);  // You expect 5 intersections

    assert(strcmp(intersections[0].id, "IntersectionA") == 0);
    assert(intersections[0].capacity == 1);
    assert(intersections[0].available == 1);

    assert(strcmp(intersections[3].id, "IntersectionD") == 0);
    assert(intersections[3].capacity == 3);

    printf("test_getIntersections passed\n");
}

int main() {
    TrainEntry trains[LINE_MAX];
    int trainCount = getTrains(trains);

    // Print individual train entries
    printf("Train Entries:\n");
    for (int i = 0; i < trainCount; i++) {
        for (int j = 0; j < trains[i].routeLength; j++) {
            printf("%s%s", trains[i].route[j], (j < trains[i].routeLength - 1) ? " -> " : "");
        }
    }

    // Print all train entries
    IntersectionEntry intersections[LINE_MAX];
    int intersectionCount = getIntersections(intersections);
    // Print individual intersection entries
    printf("Intersection Entries:\n");
    for (int i = 0; i < intersectionCount; i++) {
        printf("Intersection ID: %s, Capacity: %d, Available: %d\n", intersections[i].id, intersections[i].capacity, intersections[i].available);
    }
    // Run unit tests

    test_getTrains();
    test_getIntersections();
    printf("All unit tests passed.\n");
    return 0;
}
