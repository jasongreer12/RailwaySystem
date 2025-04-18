/*
Author: Jarett Woodard
Group: B
Email: jarett.woodard@okstate.edu
Date: 4.12.2025
*/

#ifndef CSV_LOGGER_H
#define CSV_LOGGER_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "../Basic_IPC_Workflow/ipc.h"
#include "../parser/parser.h"
#include "../Shared_Memory_Setup/Memory_Segments.h"
#include "../parser/parser.h"

// Forward declarations of structs used in CsvLogData
// typedef struct SharedIntersection SharedIntersection;
// typedef struct TrainEntry TrainEntry;

// Definition of CsvLogData struct
typedef struct {
    int train_id;
    const char* intersection_id;
    const char* action;
    const char* status;
    pid_t pid;
    const char* error_msg;
    const SharedIntersection* resource_state;
    const TrainEntry* train_state;
    int current_position;
    bool has_deadlock;
    int node_count;
    const char* cycle_path;
    const char* edge_type;
} CsvLogData;

/*
Column layout:

SYSTEM COLUMNS (audomatic)
0.  sys_time
1.  calling_file
2.  calling_function

PROVIDED FROM CALLER:
3.  train_id
4.  intersection_id
5.  action
6.  status
7.  pid

Function specific
8.  error_msg
9.  resource_state **struct
10. train_state, **struct 
11. current_position,
12. has_deadlock,
13. node_count,
14. cycle_path, 
15. edge_type
 */

FILE* csv_logger_init(void);

int LOG_CSV_impl(const char* calling_file, 
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

#define LOG_CSV(...) \
    LOG_CSV_impl(__FILE__, __func__, __VA_ARGS__)

int log_train_event_csv(FILE* file, const char* csv_data, 
                       const char* calling_file, const char* calling_func);

void csv_logger_close(void);

#endif // CSV_LOGGER_H