#ifndef CSV_LOGGER_H
#define CSV_LOGGER_H

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include "../Basic_IPC_Workflow/ipc.h"
#include "../parser/parser.h"
#include "../Shared_Memory_Setup/Memory_Segments.h"

/**
 * CSV Structure:
 * 
 * [System Columns - Automatically Added]
 * - sys_time: Nanosecond precision timestamp
 * - calling_file: Source file
 * - calling_function: Function name
 * 
 * [Event Data - Fixed Columns]
 * - train_id: Train identifier
 * - intersection_id: Intersection name
 * - action: Operation (ACQUIRE/RELEASE/etc)
 * - status: Result (GRANT/DENY/OK/FAIL)
 * - pid: Process ID
 * - error_msg: Error details if any
 * 
 * [State Data - JSON Formatted]
 * - resource_state: Shared memory state
 * - deadlock_info: Graph state
 * - train_state: Route info
 */

/**
 * Initialize CSV logger with new timestamped file
 */
FILE* csv_logger_init(void);

/**
 * Log event with full state information
 * Handles all struct-to-JSON conversion internally
 * @param file Open CSV file
 * @param train_id Train number
 * @param intersection_id Intersection name
 * @param action Operation being performed
 * @param status Result status
 * @param pid Process ID
 * @param error_msg Error description
 * @param resource_state SharedIntersection pointer (NULL ok)
 * @param train_state TrainEntry pointer (NULL ok)
 * @param current_position Position in route
 * @param has_deadlock Deadlock detected flag
 * @param node_count Graph node count
 * @param cycle_path Deadlock cycle description
 * @param edge_type Graph edge type
 * @param lock_time_ns Lock timing
 * @param failed_attempts Failed lock count
 */
int log_train_event_csv_ex_impl(FILE* file, 
                          const char* calling_file,
                          const char* calling_func,
                          int train_id,
                          const char* intersection_id,
                          const char* action,
                          const char* status,
                          pid_t pid,
                          const char* error_msg,
                          const SharedIntersection* resource_state,
                          const TrainEntry* train_state,
                          int current_position,
                          bool has_deadlock,
                          int node_count,
                          const char* cycle_path,
                          const char* edge_type,
                          long lock_time_ns,
                          int failed_attempts);

#define log_train_event_csv_ex(file, ...) \
    log_train_event_csv_ex_impl(file, __FILE__, __func__, __VA_ARGS__)

/**
 * Legacy logging function - prefer log_train_event_csv_ex
 */
int log_train_event_csv(FILE* file, const char* csv_data, 
                       const char* calling_file, const char* calling_func);

#define LOG_CSV(file, data) log_train_event_csv(file, data, __FILE__, __func__)

/**
 * Close the CSV logger
 */
void csv_logger_close(FILE* file);

#endif /* CSV_LOGGER_H */