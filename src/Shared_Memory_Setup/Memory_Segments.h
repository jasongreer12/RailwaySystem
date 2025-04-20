// Memory_Segments.h
// Author Steve Kuria
// Group B
// skuria@okstate.edu
// 4-4-2025
// This header file defines a shared memory structure for intersections—comprising a mutex, a semaphore pointer, capacity, and semaphore name—and declares functions to initialize and clean up this shared memory resource.
// 4-11-25: Created functions to track held intersections
#ifndef MEMORY_SEGMENTS_H
#define MEMORY_SEGMENTS_H

#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>

#define NUM_INTERSECTIONS 5
#define MAX_TRAINS 10

typedef struct {
    pthread_mutex_t mutex;  // guards this struct’s fields
    sem_t *semaphore;       // Pointer to named semaphore
    int capacity;           // max concurrent holders
    char semName[32];

    //Resource tracking
    int held_count;                 // how many trains currently holding
    int holders[MAX_TRAINS];        // train IDs holding this intersection

    int wait_count;                 // how many trains waiting
    int wait_queue[MAX_TRAINS];     // train IDs waiting in FIFO
} SharedIntersection;

// typedef struct {
//     pthread_mutex_t time_mutex;  // protect sim_time
//     int             sim_time;    // in seconds
// } TimeKeeper;

// Function declarations
SharedIntersection* init_shared_memory(const char *shm_name, size_t *shm_size);
void destroy_shared_memory(SharedIntersection *shared_intersections, const char *shm_name, size_t shm_size);

// Functions for tracking
int  add_holder     (SharedIntersection *shared, int idx, int train_id);
int  remove_holder  (SharedIntersection *shared, int idx, int train_id);
void enqueue_waiter (SharedIntersection *shared, int idx, int train_id);
int  dequeue_waiter (SharedIntersection *shared, int idx);

// Timekeeping functions
// TimeKeeper* init_time (const char *shm_name, size_t *shm_size);
// void       destroy_time (TimeKeeper   *shared, const char *shm_name, size_t shm_size);
// int        increment_time      (TimeKeeper   *shared, int delta);
// int        get_sim_time        (TimeKeeper   *shared);

#endif // MEMORY_SEGMENTS_H
