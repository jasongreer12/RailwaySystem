#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "csv_logger.h"

// Mock data structures for testing
static SharedIntersection mock_intersection = {
    .capacity = 2,
    .held_count = 1,
    .wait_count = 1,
    .holders = {1, 0, 0},
    .wait_queue = {2, 0, 0},
    .semName = "/sem_test"
};

static TrainEntry mock_train = {
    .id = "Train1",
    .route = {"IntersectionA", "IntersectionB", "IntersectionC"},
    .routeLength = 3
};

// Test different logging scenarios
void test_acquire_scenario(FILE* file) {
    // Test successful acquisition
    log_train_event_csv_ex(file,
        1,                    // train_id
        "IntersectionA",      // intersection_id
        "ACQUIRE",            // action
        "GRANT",             // status
        getpid(),            // pid
        NULL,                // error_msg
        &mock_intersection,  // resource_state
        &mock_train,         // train_state
        0,                   // current_position
        false,              // has_deadlock
        3,                  // node_count
        NULL,               // cycle_path
        "REQUEST",          // edge_type
        50000,             // lock_time_ns
        0);                // failed_attempts

    usleep(100000); // 100ms delay

    // Test denied acquisition
    mock_intersection.held_count = 2; // at capacity
    log_train_event_csv_ex(file,
        2,                    // train_id
        "IntersectionA",      // intersection_id
        "ACQUIRE",            // action
        "DENY",              // status
        getpid(),            // pid
        "Intersection at capacity", // error_msg
        &mock_intersection,   // resource_state
        NULL,                // train_state (omitted)
        0,                   // current_position
        false,              // has_deadlock
        3,                  // node_count
        NULL,               // cycle_path
        "REQUEST",          // edge_type
        75000,             // lock_time_ns
        1);                // failed_attempts
}

void test_deadlock_scenario(FILE* file) {
    log_train_event_csv_ex(file,
        1,                    // train_id
        "IntersectionB",      // intersection_id
        "ACQUIRE",            // action
        "DENY",              // status
        getpid(),            // pid
        "Deadlock detected",  // error_msg
        &mock_intersection,   // resource_state
        &mock_train,         // train_state
        1,                   // current_position
        true,               // has_deadlock
        4,                  // node_count
        "Train1->A->Train2->B->Train1", // cycle_path
        "REQUEST",          // edge_type
        100000,            // lock_time_ns
        2);               // failed_attempts
}

int main() {
    // Initialize CSV logger
    FILE* csv_file = csv_logger_init();
    if (!csv_file) {
        fprintf(stderr, "Failed to initialize CSV logger\n");
        return 1;
    }
    printf("CSV logger initialized successfully\n");

    // Run test scenarios
    test_acquire_scenario(csv_file);
    test_deadlock_scenario(csv_file);
    
    // Close logger
    csv_logger_close(csv_file);
    printf("CSV logger closed\n");

    return 0;
}