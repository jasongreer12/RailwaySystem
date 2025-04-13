#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "csv_logger.h"
#include "../Basic_IPC_Workflow/intersection_locks.h"
#include "../Shared_Memory_Setup/Memory_Segments.h"

// Test intersection-specific logging
void test_intersection_logging(FILE* file) {
    // Mock intersection data
    SharedIntersection mock_intersection = {
        .capacity = 2,
        .held_count = 1,
        .wait_count = 1,
        .holders = {1, 0, 0},
        .wait_queue = {2, 0, 0},
        .semName = "/sem_test"
    };

    // Test successful intersection acquisition
    log_train_event_csv_ex(file,
        1,                    // train_id
        "IntersectionA",      // intersection_id
        "ACQUIRE",            // action
        "GRANT",             // status
        getpid(),            // pid
        NULL,                // error_msg
        &mock_intersection,  // resource_state
        NULL,                // train_state
        0,                   // current_position
        false,              // has_deadlock
        3,                  // node_count
        NULL,               // cycle_path
        "REQUEST",           // edge_type
        50000,              // lock_time_ns
        0);                 // failed_attempts

    usleep(50000); // 50ms delay

    // Simulate intersection at capacity
    mock_intersection.held_count = 2;
    
    // Test denied acquisition due to capacity
    log_train_event_csv_ex(file,
        2,                    // train_id
        "IntersectionA",      // intersection_id
        "ACQUIRE",            // action
        "DENY",              // status
        getpid(),            // pid
        "At capacity",       // error_msg
        &mock_intersection,  // resource_state
        NULL,                // train_state
        0,                   // current_position
        false,              // has_deadlock
        3,                  // node_count
        NULL,               // cycle_path
        "REQUEST",           // edge_type
        75000,              // lock_time_ns
        1);                 // failed_attempts
}

// Only needed if running standalone
#ifdef TEST_STANDALONE
int main() {
    FILE* csv_file = csv_logger_init();
    if (!csv_file) {
        fprintf(stderr, "Failed to initialize CSV logger\n");
        return 1;
    }
    
    test_intersection_logging(csv_file);
    
    csv_logger_close(csv_file);
    return 0;
}
#endif