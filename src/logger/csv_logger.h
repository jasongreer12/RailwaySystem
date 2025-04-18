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

/*
LOG_CSV Parameters
0   int train_id; Null = 0
1   const char* intersection_id; null = manual or NULL
2   const char* action; manual or NULL
3   const char* status; if variable manual or NULL
4   pid_t pid; never null
5   const char* error_msg; null = manual or NULL
6   const SharedIntersection* resource_state; null = NULL
7   const TrainEntry* train_state; null = NULL
8   int current_position; null = 0
9   bool has_deadlock; null = false
10  int node_count; null = 0
11  const char* cycle_path; null = manual or NULL
12  const char* edge_type; null = manual or NULL

To utilize, replace the elements of the LOG_CSV function with the appropriate local variables.
indeces:0,        1,                   2,         3,        4,    5,    6,    7, 8,     9,10,          11,   12)
LOG_CSV(0, "SYSTEM", "INIT_INTERSECTION", "SUCCESS", getpid(), NULL, NULL, NULL, 0, false, 0, NULL, NULL);
*/

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