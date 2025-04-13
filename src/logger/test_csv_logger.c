#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "csv_logger.h"

// Test function to simulate different parts of the system
void test_function1(FILE* file) {
    LOG_CSV(file, "step1,Train1,IntersectionA,ACQUIRE,PENDING");
    usleep(100000); // 100ms delay to show time difference
    LOG_CSV(file, "step2,Train1,IntersectionA,ACQUIRE,GRANT");
}

void test_function2(FILE* file) {
    LOG_CSV(file, "step1,Train2,IntersectionB,ACQUIRE,PENDING");
    usleep(50000); // 50ms delay
    LOG_CSV(file, "step2,Train2,IntersectionB,ACQUIRE,DENY");
}

int main() {
    // Initialize CSV logger
    FILE* csv_file = csv_logger_init();
    if (!csv_file) {
        fprintf(stderr, "Failed to initialize CSV logger\n");
        return 1;
    }
    printf("CSV logger initialized successfully\n");

    // Test basic logging
    LOG_CSV(csv_file, "start,,,INIT,OK");
    printf("Logged initialization entry\n");

    // Test multiple functions writing to the log
    test_function1(csv_file);
    test_function2(csv_file);
    
    // Test missing fields
    LOG_CSV(csv_file, "step3,,,,ERROR");
    
    // Test empty data
    LOG_CSV(csv_file, "");

    // Close the logger
    csv_logger_close(csv_file);
    printf("CSV logger closed\n");

    return 0;
}