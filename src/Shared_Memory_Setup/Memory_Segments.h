// Memory_Segments.h
// Author Steve Kuria
// Group B
// skuria@okstate.edu
// 4-4-2025
// This header file defines a shared memory structure for intersections—comprising a mutex, a semaphore pointer, capacity, and semaphore name—and declares functions to initialize and clean up this shared memory resource.
#ifndef MEMORY_SEGMENTS_H
#define MEMORY_SEGMENTS_H

#include <pthread.h>
#include <semaphore.h>

#define NUM_INTERSECTIONS 5

typedef struct {
    pthread_mutex_t mutex;
    sem_t *semaphore;       // Pointer to named semaphore
    int capacity;
    char semName[32];
} SharedIntersection;

// Function declarations
SharedIntersection* init_shared_memory(const char *shm_name, size_t *shm_size);
void destroy_shared_memory(SharedIntersection *shared_intersections, const char *shm_name, size_t shm_size);

#endif // MEMORY_SEGMENTS_H
