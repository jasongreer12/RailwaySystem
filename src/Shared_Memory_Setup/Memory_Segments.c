// Memory_Segments.c
// Author Steve Kuria
// Group B
// skuria@okstate.edu
// 4-3-2025
// This code initializes a shared memory segment containing multiple intersection structures—with each structure configured with a mutex and semaphore—and provides functions to set up and clean up these resources using POSIX shared memory APIs.
// 4-11-25: Created intiialized functions to track intersections
#include "Memory_Segments.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// Function to initialize shared memory and intersections
SharedIntersection* init_shared_memory(const char *shm_name, size_t *shm_size) {
    int shm_fd;
    SharedIntersection *shared_intersections;

    *shm_size = sizeof(SharedIntersection) * NUM_INTERSECTIONS;

    shm_unlink(shm_name);  // Clean old shm if it exists

    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return NULL;
    }

    if (ftruncate(shm_fd, *shm_size) == -1) {
        perror("ftruncate");
        close(shm_fd);
        return NULL;
    }

    shared_intersections = mmap(NULL, *shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_intersections == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return NULL;
    }

    for (int i = 0; i < NUM_INTERSECTIONS; i++) {

        if (pthread_mutex_init(&shared_intersections[i].mutex, NULL) != 0) {
            perror("pthread_mutex_init");
            return NULL;
        }

        shared_intersections[i].capacity = (i % 2 == 0) ? 1 : 3;

        snprintf(shared_intersections[i].semName, sizeof(shared_intersections[i].semName),
                 "/sem_intersection_%d", i);

        sem_unlink(shared_intersections[i].semName); // In case it already exists

        shared_intersections[i].semaphore = sem_open(
            shared_intersections[i].semName,
            O_CREAT,
            0666,
            shared_intersections[i].capacity
        );

        if (shared_intersections[i].semaphore == SEM_FAILED) {
            perror("sem_open");
            return NULL;
        }

        //Set capacity and name
        SharedIntersection *si = &shared_intersections[i];
        si->capacity = (i % 2 == 0) ? 1 : 3;
        snprintf(si->semName, sizeof(si->semName), "/sem_intersection_%d", i);

        // initialize tracking arrays
        si->held_count  = 0;
        si->wait_count  = 0;
        memset(si->holders,    0, sizeof(si->holders));
        memset(si->wait_queue, 0, sizeof(si->wait_queue));
    }

    close(shm_fd); // Close the file descriptor (not the memory)
    return shared_intersections;
}

// Function to clean up resources
void destroy_shared_memory(SharedIntersection *shared_intersections, const char *shm_name, size_t shm_size) {
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        if (pthread_mutex_destroy(&shared_intersections[i].mutex) != 0) {
            perror("pthread_mutex_destroy");
        }
        if (sem_close(shared_intersections[i].semaphore) == -1) {
            perror("sem_close");
        }
        if (sem_unlink(shared_intersections[i].semName) == -1) {
            perror("sem_unlink");
        }
    }

    if (munmap(shared_intersections, shm_size) == -1) {
        perror("munmap");
    }

    if (shm_unlink(shm_name) == -1) {
        perror("shm_unlink");
    }

}

//Tracking functions

//Attempts to add train_id as a holder of intersection idx. Returns 1 if added, 0 if at capacity
int add_holder(SharedIntersection *shared, int idx, int train_id) {
    SharedIntersection *si = &shared[idx];
    pthread_mutex_lock(&si->mutex);
    if (si->held_count < si->capacity) {
        si->holders[si->held_count++] = train_id;
        pthread_mutex_unlock(&si->mutex);
        return 1;
    }
    pthread_mutex_unlock(&si->mutex);
    return 0;
}

//remove train_id from holders. Returns 1 on sucess, 0 if not found 

int remove_holder(SharedIntersection *shared, int idx, int train_id) {
    SharedIntersection *si = &shared[idx];
    int found = 0;
    pthread_mutex_lock(&si->mutex);
    for (int i = 0; i < si->held_count; i++) {
        if (si->holders[i] == train_id) {
            // shift left
            for (int j = i; j < si->held_count - 1; j++) {
                si->holders[j] = si->holders[j+1];
            }
            si->held_count--;
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&si->mutex);
    return found;
}

//Enqueues a waiting train. Caller mist handle capacity of wait_queue

void enqueue_waiter(SharedIntersection *shared, int idx, int train_id) {
    SharedIntersection *si = &shared[idx];
    pthread_mutex_lock(&si->mutex);
    if (si->wait_count < MAX_TRAINS) {
        si->wait_queue[si->wait_count++] = train_id;
    } else {
        fprintf(stderr, "Warning: wait_queue full on intersection %d\n", idx);
    }
    pthread_mutex_unlock(&si->mutex);
}

// Dequeues the oldest waiting tain, returns -1 if none

int dequeue_waiter(SharedIntersection *shared, int idx) {
    SharedIntersection *si = &shared[idx];
    int next = -1;
    pthread_mutex_lock(&si->mutex);
    if (si->wait_count > 0) {
        next = si->wait_queue[0];
        // shift left
        for (int i = 0; i < si->wait_count - 1; i++) {
            si->wait_queue[i] = si->wait_queue[i+1];
        }
        si->wait_count--;
    }
    pthread_mutex_unlock(&si->mutex);
    return next;
}