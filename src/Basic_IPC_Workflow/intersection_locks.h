// intersection_locks.h
// Author: Jake Pinell
// Group: B
// Email: jpinell@okstate.edu
// Date: 4-4-2025
// Implementation of mutex and semaphore locks for railway intersections

#ifndef INTERSECTION_LOCKS_H
#define INTERSECTION_LOCKS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

// max length of intersection name (increased to accommodate "/sem_" prefix)
#define MAX_NAME_LENGTH 32

// struct to represent an intersection with synchronization primitives
typedef struct {
    char name[MAX_NAME_LENGTH];      // Intersection name
    int capacity;                    // Max number of trains allowed
    pthread_mutex_t mutex;           // Mutex (capacity = 1)
    sem_t *semaphore;                // Semaphore (capacity > 1)
    char semName[MAX_NAME_LENGTH];   // Unique name for semaphore
} Intersection;


// Initialize a mutex for an intersection with capacity 1
// Returns true on success, false on failure
bool init_mutex_lock(Intersection *intersection);

// Initialize a semaphore for an intersection with capacity > 1
// Returns true on success, false on failure
bool init_semaphore_lock(Intersection *intersection);

// Acquire a lock for an intersection based on its capacity
// Returns 0 on success, -1 on failure
int acquire_lock(Intersection *intersection);

// Release a lock for an intersection based on its capacity
// Returns 0 on success, -1 on failure
int release_lock(Intersection *intersection);

// Clean up an intersection's locks
void cleanup_locks(Intersection *intersection);

#endif // INTERSECTION_LOCKS_H