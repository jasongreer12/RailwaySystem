// Author Steve Kuria
// skuria@okstate.edu
// 4-3-2025
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#define SHM_NAME "/intersection_shm"
#define NUM_INTERSECTIONS 5  

typedef struct {
    pthread_mutex_t mutex;
    sem_t *semaphore;       // Semaphore pointer
    int capacity;           // Semaphore capacity
    char semName[32];       // Unique name for the semaphore
} Intersection;

int main() {
    int shm_fd;
    Intersection *shared_intersections;

    // Shared memory object
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Set size of shared memory
    if (ftruncate(shm_fd, sizeof(Intersection) * NUM_INTERSECTIONS) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    // Map shared memory into the address space
    shared_intersections = mmap(NULL, sizeof(Intersection) * NUM_INTERSECTIONS, 
                                PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_intersections == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Initialize each intersection with a mutex and semaphore
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
    
        if (pthread_mutex_init(&shared_intersections[i].mutex, NULL) != 0) {
            perror("pthread_mutex_init");
            exit(EXIT_FAILURE);
        }

        // Set capacity
        shared_intersections[i].capacity = (i % 2 == 0) ? 1 : 3;

        snprintf(shared_intersections[i].semName, sizeof(shared_intersections[i].semName), "/sem_intersection_%d", i);

        // Unlink in case a semaphore with this name already exists
        sem_unlink(shared_intersections[i].semName);

        // Create the named semaphore
        shared_intersections[i].semaphore = sem_open(shared_intersections[i].semName, O_CREAT, 0666, shared_intersections[i].capacity);
        if (shared_intersections[i].semaphore == SEM_FAILED) {
            perror("sem_open");
            exit(EXIT_FAILURE);
        }
    }

    printf("Shared memory initialized.\n");


    // Cleanup: Destroy mutexes and close/unlink named semaphores
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

    // Unmap shared memory
    if (munmap(shared_intersections, sizeof(Intersection) * NUM_INTERSECTIONS) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    // Close shared memory file descriptor
    if (close(shm_fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    // Unlink shared memory object
    if (shm_unlink(SHM_NAME) == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }

    printf("Cleanup complete.\n");

    return 0;
}
