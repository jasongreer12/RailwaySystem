// intersection_locks.h
// Author: Jake Pinell
// Group: B
// Email: jpinell@okstate.edu
// Date: 4-18-2025
// Implementation of mutex and semaphore locks for railway intersections, intersections now track which trains are currently passing through.


#ifndef INTERSECTION_LOCKS_H
#define INTERSECTION_LOCKS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include "../logger/csv_logger.h"

// max length of intersection name (increased to accommodate "/sem_" prefix)
#define MAX_NAME_LENGTH 32

// struct to represent an intersection
typedef struct {
    char name[MAX_NAME_LENGTH];    
    int capacity;                  
    
    pthread_mutex_t mutex;
    
    sem_t *semaphore;
    char semName[MAX_NAME_LENGTH];
    
    int train_ids[MAX_TRAINS];   
    int num_trains;             
    pthread_mutex_t tracker_mutex; 
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

// Check if a train is currently in the intersection
bool is_train_in_intersection(Intersection *intersection, int train_id);

#endif // INTERSECTION_LOCKS_H
