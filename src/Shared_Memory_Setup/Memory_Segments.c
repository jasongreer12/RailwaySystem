// Memory_Segments.c
// Author Steve Kuria
// Group B
// skuria@okstate.edu
// 4-3-2025
// This code initializes a shared memory segment containing multiple intersection structures—with each structure configured with a mutex and semaphore—and provides functions to set up and clean up these resources using POSIX shared memory APIs.
#include "Memory_Segments.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

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
    }

    close(shm_fd); // Close the file descriptor (not the memory)
    printf("Shared memory initialized.\n");
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

    printf("Shared memory and resources cleaned up.\n");
}
