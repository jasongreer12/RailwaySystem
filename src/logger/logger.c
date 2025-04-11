// logger.c
// Jason Greer
// jason.greer@okstate.edu
// 4/10/25
// Basic logger implementation for simulation.log 

#include "logger.h"
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int log_fd = -1;

void log_init(const char *filename) {
    /* Open the log file for writing.
     * O_TRUNC ensures the file starts fresh for each run.
     * O_CREAT creates the file if it doesn't exist.
     * O_WRONLY and O_APPEND are used for writing and appending.
     */
    log_fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY | O_APPEND, 0666);
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
}
