// Jason Greer
// jason.greer@okstate.edu
// 4/10/25
// GROUP: B
// logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>
#include "../Basic_IPC_Workflow/fake_sec.h" // for getFakeTime()

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

/*
original macro
#define LOG_SERVER(fmt, ...) log_event("SERVER", fmt, ##__VA_ARGS__) //adjusting macro to incorporate time in log
Updated by jarett on 4.19 to include time string
*/
#define LOG_SERVER(fmt, ...) do { \
    char comp[64];\
    snprintf(comp, sizeof(comp), "%s SERVER", getFakeTime());\
    log_event(comp, fmt, ##__VA_ARGS__);\
} while(0)

//comp size increased to 32 to accommodate addition of time string
#define LOG_TRAIN(id, fmt, ...) do { \
    char comp[64]; \
    snprintf(comp, sizeof(comp), "%s TRAIN%d", getFakeTime(), (id)); \
    log_event(comp, fmt, ##__VA_ARGS__); \
} while(0)

#endif /* LOGGER_H */