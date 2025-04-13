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

FILE* csv_logger_init(void);

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
                          const char* edge_type);

#define log_train_event_csv_ex(file, ...) \
    log_train_event_csv_ex_impl(file, __FILE__, __func__, __VA_ARGS__)

int log_train_event_csv(FILE* file, const char* csv_data, 
                       const char* calling_file, const char* calling_func);

#define LOG_CSV(file, data) log_train_event_csv(file, data, __FILE__, __func__)

void csv_logger_close(FILE* file);

#endif /* CSV_LOGGER_H */