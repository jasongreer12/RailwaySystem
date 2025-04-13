#define _POSIX_C_SOURCE 199309L
#include "csv_logger.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

FILE* csv_logger_init(void) {
    time_t now;
    struct tm *tm_info;
    char filename[64];
    FILE *file;

    // Get current time
    time(&now);
    tm_info = localtime(&now);

    // Create filename with timestamp
    strftime(filename, sizeof(filename), "train_run_%m%d%Y_%H%M%S.csv", tm_info);

    // Open file
    file = fopen(filename, "w");
    if (!file) {
        return NULL;
    }

    // Write CSV header
    /*Indeces and definitions:
    0: sys_time: Timestamp from simulation - This is provided in the print function and not sent as an argument
    1: sim_time: Timestamp from simulation
    2: calling_function: Function that called the logger
    3: step: What step in the calling function
    4: Train ID: Name of the train
    5: Train Location: Intersection where train is located
    6: Action: Action being performed (ACQUIRE, RELEASE, etc)
    7: Status: Result of the action (GRANT, DENY, OK, FAIL)
    */

    fprintf(file, "sys_time,calling_function,step,Train ID,Intersection,Action,Status\n");
    fflush(file);

    return file;
}

int log_train_event_csv(FILE* file, 
                       const char* train_id, 
                       const char* intersection_id, 
                       const char* action, 
                       const char* status,
                       const char* timestamp) {
    if (!file || !intersection_id || !action || !status) {
        return -1;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    // Format timestamp with nanosecond precision
    char precise_time[32];
    struct tm *tm_info = localtime(&ts.tv_sec);
    strftime(precise_time, sizeof(precise_time), "%Y-%m-%d %H:%M:%S", tm_info);
    char sys_time[48];
    snprintf(sys_time, sizeof(sys_time), "%s.%09ld", precise_time, ts.tv_nsec);

    // Write CSV line with precise timestamp
    fprintf(file, "%s,%d,%s,%s,%s\n",
            sys_time,
            train_id,
            intersection_id,
            action,
            status);
    
    // Ensure it's written to disk
    fflush(file);
    
    return 0;
}

void csv_logger_close(FILE* file) {
    if (file) {
        fclose(file);
    }
}