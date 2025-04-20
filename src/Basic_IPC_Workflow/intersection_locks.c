// intersection_locks.c
// Author: Jake Pinell
// Group: B
// Email: jpinell@okstate.edu
// Date: 4-19-2025
// Implementation of locks for intersections in the railway system. Initializes mutexes and semaphores for intersections based on their train capacity.
// Implemented acquire and release functions for intersection locks. Cleans up all intersection locks after use.

#include "intersection_locks.h"
#include <fcntl.h>
#include "fake_sec.h"

//local time functions. Saves by not have to declare the
//shared intersection every time we need to call the time functions.
// void increaseTime(int inc){
//     SharedIntersection *si = &shared_intersections[0];
//     setFakeSec(inc);
// }

// char *getTime(){
//     SharedIntersection *si = &shared_intersections[0];
//     char *timeString = malloc(11);
//     snprintf(timeString, 11, "%s", getFakeTime());
//     return timeString;
// }

// Initialize mutex for intersection with capacity 1
bool init_mutex_lock(Intersection *intersection) {
    if (!intersection) {
        fprintf(stderr, "Invalid intersection pointer\n");
        return false;
    }
    
    // Initialize mutex with default attributes
    if (pthread_mutex_init(&intersection->mutex, NULL) != 0) {
        perror("Failed to initialize mutex");
        return false;
    }
    
    printf("%s Initialized mutex for intersection %s (capacity 1)\n", getFakeTime(), intersection->name);
    return true;
}

// Initialize semaphore for intersection with capacity > 1
bool init_semaphore_lock(Intersection *intersection) {
    if (!intersection || intersection->capacity <= 0) {
        fprintf(stderr, "Invalid intersection or capacity\n");
        return false;
    }
    
    // Generate a name for the semaphore
    snprintf(intersection->semName, MAX_NAME_LENGTH, "/sem_%.26s", intersection->name);
    sem_unlink(intersection->semName);
    
    // Create the semaphore, capacity = value
    intersection->semaphore = sem_open(
        intersection->semName,
        O_CREAT,
        0666,
        intersection->capacity
    );
    
    if (intersection->semaphore == SEM_FAILED) {
        perror("Failed to initialize semaphore");
        return false;
    }
    
    printf("%s Initialized semaphore for intersection %s (capacity %d)\n",getFakeTime(), intersection->name, intersection->capacity);
    return true;
}

// Acquire a lock for an intersection
int acquire_lock(Intersection *intersection) {
    if (!intersection) {
        fprintf(stderr, "Invalid intersection pointer\n");
        return -1;
    }
    
    int result = 0;
    setFakeSec(1);
    if (intersection->capacity == 1) {
        // For capacity 1 use mutex
        result = pthread_mutex_lock(&intersection->mutex);
        if (result != 0) {
            perror("Failed to acquire mutex lock");
            return -1;
        }
        printf("%s Acquired mutex lock for intersection %s\n", getFakeTime(), intersection->name);
    } else {
        // For capacity > 1 use semaphore
        result = sem_wait(intersection->semaphore);
        if (result != 0) {
            perror("Failed to acquire semaphore lock");
            return -1;
        }
        printf("%s Acquired semaphore lock for intersection %s\n", getFakeTime(), intersection->name);
    }
    
    return 0;
}

// Release a lock for an intersection
int release_lock(Intersection *intersection) {
    if (!intersection) {
        fprintf(stderr, "Invalid intersection pointer\n");
        return -1;
    }
    
    int result = 0;
    setFakeSec(1);
    
    if (intersection->capacity == 1) {
        // For capacity 1 use mutex
        result = pthread_mutex_unlock(&intersection->mutex);
        if (result != 0) {
            perror("Failed to release mutex lock");
            return -1;
        }
        setFakeSec(1); // Increment time by 1 second
        printf("%s Released mutex lock for intersection %s\n", getFakeTime(), intersection->name);
    } else {
        // For capacity > 1 use semaphore
        result = sem_post(intersection->semaphore);
        if (result != 0) {
            perror("Failed to release semaphore lock");
            return -1;
        }
        setFakeSec(1);
        printf("%s Released semaphore lock for intersection %s\n", getFakeTime(), intersection->name);
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
