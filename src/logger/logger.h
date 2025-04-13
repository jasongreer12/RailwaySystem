// Jason Greer
// jason.greer@okstate.edu
// 4/10/25
// GROUP: B
// logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

void log_init(const char *filename, int truncate);

/* Write a log event.
 * The 'component' is a string that identifies the source (e.g., "SERVER", "TRAIN1").
 * The fmt and additional arguments are similar to printf.
 */
void log_event(const char *component, const char *fmt, ...);

/* Close the log file. */
void log_close(void);

/* Convenience macros for common components.
 */
#define LOG_SERVER(fmt, ...) log_event("SERVER", fmt, ##__VA_ARGS__)
#define LOG_TRAIN(id, fmt, ...) do { \
    char comp[16]; \
    snprintf(comp, sizeof(comp), "TRAIN%d", (id)); \
    log_event(comp, fmt, ##__VA_ARGS__); \
} while(0)

#endif /* LOGGER_H */