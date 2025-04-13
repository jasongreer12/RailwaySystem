#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "csv_logger.h"

// Example specialized data for test_csv_logger.c
typedef struct {
    double cpu_usage;
    long memory_used;
    int active_threads;
} SystemMetrics;

// Test basic logging functionality
void test_basic_logging(FILE* file) {
    SystemMetrics metrics = {
        .cpu_usage = 45.2,
        .memory_used = 1048576,
        .active_threads = 4
    };

    // Log system startup with metrics in appropriate field
    log_train_event_csv_ex(file,
        0,                    // train_id
        "SYSTEM",            // intersection_id
        "STARTUP",           // action
        "OK",               // status
        getpid(),           // pid
        NULL,               // error_msg
        NULL,               // resource_state
        NULL,               // train_state
        0,                  // current_position
        false,             // has_deadlock
        0,                  // node_count
        NULL,               // cycle_path
        NULL,              // edge_type
        0,                  // lock_time_ns
         0);                // failed_attempts

    usleep(100000); // 100ms delay

    // Simulate memory pressure
    metrics.memory_used *= 2;
    metrics.cpu_usage = 78.5;

    log_train_event_csv_ex(file,
        0,                    
        "SYSTEM",            
        "MONITOR",          
        "WARNING",          
        getpid(),           
        "High memory usage",
        NULL,               
        NULL,               
        0,                  
        false,             
        0,                  
        NULL,               
        NULL,              
        0,                  
         0);
}

extern void test_intersection_logging(FILE* file);

int main() {
    FILE* csv_file = csv_logger_init();
    if (!csv_file) {
        fprintf(stderr, "Failed to initialize CSV logger\n");
        return 1;
    }
    printf("CSV logger initialized successfully\n");

    test_basic_logging(csv_file);
    test_intersection_logging(csv_file);
    
    csv_logger_close(csv_file);
    printf("CSV logger closed\n");

    return 0;
}