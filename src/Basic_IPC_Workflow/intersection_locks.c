// intersection_locks.c
// Author: Jake Pinell
// Group: B
// Email: jpinell@okstate.edu
// Date: 4-4-2025
// Implementation of mutex and semaphore locks for railway intersections

#include "intersection_locks.h"
#include <fcntl.h>

// Initialize a mutex for an intersection with capacity 1
bool init_mutex_lock(Intersection *intersection) {
    if (!intersection) {
        fprintf(stderr, "Invalid intersection pointer\n");
        return false;
    }
    
    // Initialize mutex with default attributes
    if (pthread_mutex_init(&intersection->mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        return false;
    }
    
    printf("Initialized mutex for intersection %s (capacity 1)\n", intersection->name);
    return true;
}

// Initialize a semaphore for an intersection with capacity > 1
bool init_semaphore_lock(Intersection *intersection) {
    if (!intersection || intersection->capacity <= 0) {
        fprintf(stderr, "Invalid intersection or capacity\n");
        return false;
    }
    
    // Generate a unique semaphore name based on intersection name
    snprintf(intersection->semName, MAX_NAME_LENGTH, "/sem_%s", intersection->name);
    sem_unlink(intersection->semName);
    
    // Create the named semaphore with initial value = capacity
    intersection->semaphore = sem_open(
        intersection->semName,
        O_CREAT,
        0666,
        intersection->capacity
    );
    
    if (intersection->semaphore == SEM_FAILED) {
        perror("sem_open");
        return false;
    }
    
    printf("Initialized semaphore for intersection %s (capacity %d)\n",
           intersection->name, intersection->capacity);
    return true;
}

// Acquire a lock for an intersection based on its capacity
int acquire_lock(Intersection *intersection) {
    if (!intersection) {
        fprintf(stderr, "Invalid intersection pointer\n");
        return -1;
    }
    
    int result = 0;
    
    if (intersection->capacity == 1) {
        // For capacity 1, use pthread_mutex_lock
        result = pthread_mutex_lock(&intersection->mutex);
        if (result != 0) {
            perror("pthread_mutex_lock");
            return -1;
        }
        printf("Acquired mutex lock for intersection %s\n", intersection->name);
    } else {
        // For capacity > 1, use sem_wait
        result = sem_wait(intersection->semaphore);
        if (result != 0) {
            perror("sem_wait");
            return -1;
        }
        printf("Acquired semaphore lock for intersection %s\n", intersection->name);
    }
    
    return 0;
}

// Release a lock for an intersection based on its capacity
int release_lock(Intersection *intersection) {
    if (!intersection) {
        fprintf(stderr, "Invalid intersection pointer\n");
        return -1;
    }
    
    int result = 0;
    
    if (intersection->capacity == 1) {
        // For capacity 1, use pthread_mutex_unlock
        result = pthread_mutex_unlock(&intersection->mutex);
        if (result != 0) {
            perror("pthread_mutex_unlock");
            return -1;
        }
        printf("Released mutex lock for intersection %s\n", intersection->name);
    } else {
        // For capacity > 1, use sem_post
        result = sem_post(intersection->semaphore);
        if (result != 0) {
            perror("sem_post");
            return -1;
        }
        printf("Released semaphore lock for intersection %s\n", intersection->name);
    }
    
    return 0;
}

// Clean up an intersection's locks
void cleanup_locks(Intersection *intersection) {
    if (!intersection) {
        return;
    }
    
    if (intersection->capacity == 1) {
        // Clean up mutex
        pthread_mutex_destroy(&intersection->mutex);
    } else if (intersection->semaphore != NULL) {
        // Clean up semaphore
        sem_close(intersection->semaphore);
        sem_unlink(intersection->semName);
    }
    
    printf("Cleaned up locks for intersection %s\n", intersection->name);
}
