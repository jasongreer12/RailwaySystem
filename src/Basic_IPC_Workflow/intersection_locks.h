// intersection_locks.h
// Author: Jake Pinell
// Group: B
// Email: jpinell@okstate.edu
// Date: 4-20-2025
// Implementation of locks for intersections in the railway system. Initializes mutexes and counting semaphores for intersections based on their train capacity.
// Implemented acquire and release functions for intersection locks. Cleans up all intersection locks after use.
// Intersections with a train capacity of 1 are initialized with a mutex, if the capacity is > 1, the intersection is initialized with a semaphore.

#ifndef INTERSECTION_LOCKS_H
#define INTERSECTION_LOCKS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

// Max length of intersection name
#define MAX_NAME_LENGTH 32

// Struct to represent an intersection
typedef struct {
    char name[MAX_NAME_LENGTH];      // Intersection name
    int capacity;                    // Max number of trains allowed
    pthread_mutex_t mutex;           // Mutex (capacity = 1)
    sem_t *semaphore;                // Semaphore (capacity > 1)
    char semName[MAX_NAME_LENGTH];   // Unique name for semaphore
} Intersection;


// Initialize mutex for intersection with capacity 1
// Returns true on success, false on failure
bool init_mutex_lock(Intersection *intersection);

// Initialize semaphore for intersection with capacity > 1
// Returns true on success, false on failure
bool init_semaphore_lock(Intersection *intersection);

// Acquire a lock for an intersection
// Returns 0 on success, -1 on failure
int acquire_lock(Intersection *intersection);

// Release a lock for an intersection
// Returns 0 on success, -1 on failure
int release_lock(Intersection *intersection);

// Clean up an intersection's locks
void cleanup_locks(Intersection *intersection);

#endif // INTERSECTION_LOCKS_H
