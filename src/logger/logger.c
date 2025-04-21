// logger.c
// Jason Greer
// jason.greer@okstate.edu
// 4/10/25
// Basic logger implementation for simulation.log 

#include "logger.h"
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include "logger.h" // initialize log
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>  // for stat()
#include <sys/types.h> // for stat()
#include <stdbool.h>   // for bool type
#include <sys/mman.h>
#include "../Basic_IPC_Workflow/fake_sec.h" // for getFakeTime()
#include "../Shared_Memory_Setup/Memory_Segments.h" // for SharedIntersection Struct

/*
In final iteration, the logger received the log_init and log_close functions\
We also added quite a bit of logic to these functions to ensure clean acquisition
and release of resources.
*/

static int log_fd = -1;
size_t shm_size; // moved to global to reduce redundant init calls

void log_init(const char *filename, int truncate) {
    //clean old memory on server. Fixes issue from previous iterations
    if (truncate) {
        shm_unlink("/intersection_shm");
    }

    //check for existing shared memory, create new if not found
    if (!shared_intersections) {
        size_t shm_size = sizeof(SharedIntersection) * NUM_INTERSECTIONS;
        shared_intersections = init_shared_memory("/intersection_shm", &shm_size);
        if (!shared_intersections) {
            fprintf(stderr, "Failed to initialize shared memory.\n");
            exit(1);
        }
        
        //reset time values on parent process
        pthread_mutex_lock(&shared_intersections[0].mutex);
        shared_intersections[0].fakeSec = 0;
        shared_intersections[0].fakeMin = 0;
        shared_intersections[0].fakeMinSec = 0;
        shared_intersections[0].fakeHour = 0;
        pthread_mutex_unlock(&shared_intersections[0].mutex);
    }
    
    //use flags limit actions on file to create, write, append
    //truncate (clear) is allowed if specified in function call
    int flags = O_CREAT | O_WRONLY | O_APPEND;
    if (truncate) {
        flags |= O_TRUNC;
    }
    log_fd = open(filename, flags, 0666);
    if (log_fd < 0) {
        perror("open simulation.log");
    }
}

void log_event(const char *component, const char *fmt, ...) {
    if (log_fd < 0) {
        return;
    }

    char buffer[1024];
    int offset = 0;

    /* Write the component prefix. For example: "SERVER: " or "TRAIN1: " */
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s: ", component);

    /* Process the variable argument list similar to printf */
    va_list args;
    va_start(args, fmt);
    offset += vsnprintf(buffer + offset, sizeof(buffer) - offset, fmt, args);
    va_end(args);

    /* Append a newline if there's room */
    if (offset < (int)sizeof(buffer) - 1) {
        buffer[offset++] = '\n';
        buffer[offset] = '\0';
    }

    /* Write the formatted string to the log file */
    write(log_fd, buffer, strlen(buffer));
}

void log_close(void) {
    if (log_fd >= 0) {
        close(log_fd);
        log_fd = -1;
    }
    
    // Close and unlink all semaphores and destroy mutexes before attempting to close shared memory
    // Semaphores: Close
    if (shared_intersections) {
        for (int i = 0; i < NUM_INTERSECTIONS; i++) {
            if (shared_intersections[i].semaphore != SEM_FAILED && shared_intersections[i].semaphore != NULL) {
                sem_close(shared_intersections[i].semaphore);
            }
        }
        //unlink
        for (int i = 0; i < NUM_INTERSECTIONS; i++) {
            if (shared_intersections[i].semaphore != SEM_FAILED && shared_intersections[i].semaphore != NULL) {
                sem_unlink(shared_intersections[i].semName);
            }
        }

        //Mutexes: destroy
        for (int i = 0; i < NUM_INTERSECTIONS; i++) {
            pthread_mutex_destroy(&shared_intersections[i].mutex);
        }

        //unmap shared memory
        munmap(shared_intersections, sizeof(SharedIntersection) * NUM_INTERSECTIONS);
        shm_unlink("/intersection_shm");
        shared_intersections = NULL;
    }
}