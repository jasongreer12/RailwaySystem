#ifndef CSV_LOGGER_H
#define CSV_LOGGER_H

#include <stdio.h>
#include <time.h>
#include "../Basic_IPC_Workflow/ipc.h"
#include "../parser/parser.h"

/**
 * Initializes the CSV logger and creates a new CSV file with timestamp
 * Format: train_run_mmddyyyy_hhmmss.csv
 * @return FILE* pointer to the opened CSV file, NULL if error
 */
FILE* csv_logger_init(void);

/**
 * Logs train movement data to the CSV file
 * @param file Pointer to the opened CSV file
 * @param train_id ID of the train
 * @param intersection_id ID of the intersection
 * @param action Action being performed (ACQUIRE, RELEASE, etc)
 * @param status Result of the action (GRANT, DENY, OK, FAIL)
 * @param timestamp Time of the event
 * @return 0 on success, -1 on error
 */
int log_train_event_csv(FILE* file, 
                       int train_id, 
                       const char* intersection_id, 
                       const char* action, 
                       const char* status,
                       const char* timestamp);

/**
 * Closes the CSV logger
 * @param file Pointer to the opened CSV file
 */
void csv_logger_close(FILE* file);

#endif /* CSV_LOGGER_H */