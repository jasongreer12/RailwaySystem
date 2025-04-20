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
#include "logger.h" // intialize log
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>  // for stat()
#include <sys/types.h> // for stat()
#include <stdbool.h>   // for bool type
#include "../Basic_IPC_Workflow/fake_sec.h" // for getFakeTime()
#include "../Shared_Memory_Setup/Memory_Segments.h" // for SharedIntersection Struct

static int log_fd = -1;
// SharedIntersection *shared_intersections = NULL;
size_t shm_size; // moved to global to reduce redundant init calls

void log_init(const char *filename, int truncate) {
    // initialize shared memory here to reduce redundant init calls
    shared_intersections = init_shared_memory("/intersection_shm", &shm_size);
    if (!shared_intersections) {
        fprintf(stderr, "Failed to initialize shared memory.\n");
        exit(1);
    }
    
    // Set up flags for opening the file. We always open for write and append.
    int flags = O_CREAT | O_WRONLY | O_APPEND;
    // If truncate flag is set (non-zero), add O_TRUNC to always create a new file.
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
    
    if (shared_intersections) {
        destroy_shared_memory(shared_intersections, "/intersection_shm", shm_size);
        shared_intersections = NULL;
    }
}