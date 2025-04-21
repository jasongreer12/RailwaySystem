// Jason Greer
// jason.greer@okstate.edu
// 4/10/25
// GROUP: B
// logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>
#include "../Basic_IPC_Workflow/fake_sec.h" // for getFakeTime()
#include "../Shared_Memory_Setup/Memory_Segments.h" // for SharedIntersection

//GLOBAL SHARED INTERSECTIONS ARRAY AND SIZE
extern SharedIntersection *shared_intersections;
extern size_t shm_size; // moved to global to reduce redundant init calls

/*Added initialize and close log functions to centralize all log operations.*/
void log_init(const char *filename, int truncate);
void log_close(void);

/* Write a log event.
 * The 'component' is a string that identifies the source (e.g., "SERVER", "TRAIN1").
 * The fmt and additional arguments are similar to printf.
 */
void log_event(const char *component, const char *fmt, ...);



/* Convenience macros for common components.
Updated 4.20.2025
Updated macros to include calls to get timeString
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