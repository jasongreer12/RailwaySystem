// intersection_locks.c
// Author: Jake Pinell
// Group: B
// Email: jpinell@okstate.edu
// Date: 4-18-2025
// Implementation of mutex and semaphore locks for railway intersections, intersections now track which trains are currently passing through.

#include "intersection_locks.h"
#include <fcntl.h>
#include "../logger/csv_logger.h"

/*
See csv_logger.h for definitions.
Update relevant fields with local variables to pass into csv log for debugging.
LOG_CSV(0, "SYSTEM", "INIT_INTERSECTION", "SUCCESS", getpid(), NULL, NULL, NULL, 0, false, 0, NULL, NULL);
*/

// Initialize tracking for trains in the intersection
static void init_train_tracking(Intersection *intersection) {
    // Initialize tracking array with -1 (no train)
    for (int i = 0; i < MAX_TRAINS; i++) {
        intersection->train_ids[i] = -1;
    }
    intersection->num_trains = 0;
    
    // Initialize mutex for protecting access to the train tracking array
    pthread_mutex_init(&intersection->tracker_mutex, NULL);
}

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
    
    // Initialize train tracking
    init_train_tracking(intersection);
    
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
    snprintf(intersection->semName, MAX_NAME_LENGTH, "/sem_%.26s", intersection->name);
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
    
    // Initialize train tracking
    init_train_tracking(intersection);
    
    printf("Initialized semaphore for intersection %s (capacity %d)\n",
           intersection->name, intersection->capacity);
    return true;
}

// Check if a train is currently in the intersection
bool is_train_in_intersection(Intersection *intersection, int train_id) {
    if (!intersection || train_id < 0) {
        return false;
    }
    
    bool found = false;
    
    // Lock the tracker mutex
    pthread_mutex_lock(&intersection->tracker_mutex);
    
    // Check if the train is in the intersection
    for (int i = 0; i < MAX_TRAINS; i++) {
        if (intersection->train_ids[i] == train_id) {
            found = true;
            break;
        }
    }
    
    // Unlock the tracker mutex
    pthread_mutex_unlock(&intersection->tracker_mutex);
    
    return found;
}

// Add a train to the intersection
static bool add_train_to_intersection(Intersection *intersection, int train_id) {
    if (!intersection || train_id < 0) {
        return false;
    }
    
    bool added = false;
    
    // Lock the tracker mutex
    pthread_mutex_lock(&intersection->tracker_mutex);
    
    // Make sure the train isn't already in the intersection
    bool already_in = false;
    for (int i = 0; i < MAX_TRAINS; i++) {
        if (intersection->train_ids[i] == train_id) {
            already_in = true;
            break;
        }
    }
    
    if (!already_in) {
        // Find an empty slot
        for (int i = 0; i < MAX_TRAINS; i++) {
            if (intersection->train_ids[i] == -1) {
                intersection->train_ids[i] = train_id;
                intersection->num_trains++;
                added = true;
                
                LOG_CSV(0, "SYSTEM", "ADD_TRAIN", "SUCCESS", train_id, 
                       intersection->name, NULL, NULL, 0, false, 0, NULL, NULL);
                break;
            }
        }
    }
    
    // Unlock the tracker mutex
    pthread_mutex_unlock(&intersection->tracker_mutex);
    
    return added;
}

// Remove a train from the intersection
static bool remove_train_from_intersection(Intersection *intersection, int train_id) {
    if (!intersection || train_id < 0) {
        return false;
    }
    
    bool removed = false;
    
    // Lock the tracker mutex
    pthread_mutex_lock(&intersection->tracker_mutex);
    
    // Find and remove the train
    for (int i = 0; i < MAX_TRAINS; i++) {
        if (intersection->train_ids[i] == train_id) {
            intersection->train_ids[i] = -1;
            intersection->num_trains--;
            removed = true;
            
            LOG_CSV(0, "SYSTEM", "REMOVE_TRAIN", "SUCCESS", train_id, 
                   intersection->name, NULL, NULL, 0, false, 0, NULL, NULL);
            break;
        }
    }
    
    // Unlock the tracker mutex
    pthread_mutex_unlock(&intersection->tracker_mutex);
    
    return removed;
}

// Acquire a lock for an intersection based on its capacity
int acquire_lock(Intersection *intersection, int train_id) {
    if (!intersection) {
        fprintf(stderr, "Invalid intersection pointer\n");
        return -1;
    }
    
    // Check if the train is already in the intersection
    if (is_train_in_intersection(intersection, train_id)) {
        fprintf(stderr, "Train %d is already in intersection %s\n", 
                train_id, intersection->name);
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
        
        // Add the train to the intersection
        add_train_to_intersection(intersection, train_id);
        
        printf("Train %d acquired mutex lock for intersection %s\n", 
               train_id, intersection->name);
    } else {
        // For capacity > 1, use sem_wait
        result = sem_wait(intersection->semaphore);
        if (result != 0) {
            perror("sem_wait");
            return -1;
        }
        
        // Add the train to the intersection
        add_train_to_intersection(intersection, train_id);
        
        printf("Train %d acquired semaphore lock for intersection %s\n", 
               train_id, intersection->name);
    }
    
    return 0;
}

// Release a lock for an intersection based on its capacity
int release_lock(Intersection *intersection, int train_id) {
    if (!intersection) {
        fprintf(stderr, "Invalid intersection pointer\n");
        return -1;
    }
    
    // Check if the train is actually in the intersection
    if (!is_train_in_intersection(intersection, train_id)) {
        fprintf(stderr, "Train %d is not in intersection %s\n", 
                train_id, intersection->name);
        return -1;
    }
    
    int result = 0;
    
    // First remove the train from our tracking
    remove_train_from_intersection(intersection, train_id);
    
    if (intersection->capacity == 1) {
        // For capacity 1, use pthread_mutex_unlock
        result = pthread_mutex_unlock(&intersection->mutex);
        if (result != 0) {
            perror("pthread_mutex_unlock");
            return -1;
        }
        printf("Train %d released mutex lock for intersection %s\n", 
               train_id, intersection->name);
    } else {
        // For capacity > 1, use sem_post
        result = sem_post(intersection->semaphore);
        if (result != 0) {
            perror("sem_post");
            return -1;
        }
        printf("Train %d released semaphore lock for intersection %s\n", 
               train_id, intersection->name);
    }
    
    return 0;
}

// Clean up an intersection's locks
void cleanup_locks(Intersection *intersection) {
    if (!intersection) {
        return;
    }
    
    // Clean up the tracker mutex
    pthread_mutex_destroy(&intersection->tracker_mutex);
    
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
