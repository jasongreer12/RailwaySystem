#define _POSIX_C_SOURCE 199309L
#include "csv_logger.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

// Helper functions to convert structs to JSON strings
static void shared_intersection_to_json(char* buffer, size_t size, const SharedIntersection* si) {
    if (!si) {
        snprintf(buffer, size, "{}");
        return;
    }

    // Build holders array string
    char holders_str[128] = "";
    for (int i = 0; i < si->held_count; i++) {
        char temp[16];
        snprintf(temp, sizeof(temp), "%s%d", i ? "," : "", si->holders[i]);
        strcat(holders_str, temp);
    }

    // Build waiters array string
    char waiters_str[128] = "";
    for (int i = 0; i < si->wait_count; i++) {
        char temp[16];
        snprintf(temp, sizeof(temp), "%s%d", i ? "," : "", si->wait_queue[i]);
        strcat(waiters_str, temp);
    }

    snprintf(buffer, size,
        "{\"holders_count\":%d,\"wait_count\":%d,"
        "\"holders\":[%s],\"waiters\":[%s],"
        "\"lock_type\":\"%s\",\"sem_name\":\"%s\"}",
        si->held_count, si->wait_count,
        holders_str, waiters_str,
        si->capacity == 1 ? "MUTEX" : "SEMAPHORE",
        si->semName);
}

static void train_entry_to_json(char* buffer, size_t size, const TrainEntry* te, int current_pos) {
    if (!te) {
        snprintf(buffer, size, "{}");
        return;
    }

    // Build route array string
    char route_str[512] = "";
    for (int i = 0; i < te->routeLength; i++) {
        char temp[64];
        snprintf(temp, sizeof(temp), "%s\"%s\"", 
                i ? "," : "", te->route[i]);
        strcat(route_str, temp);
    }

    snprintf(buffer, size,
        "{\"route_length\":%d,\"position\":%d,\"route\":[%s]}",
        te->routeLength, current_pos, route_str);
}

static void deadlock_info_to_json(char* buffer, size_t size, bool has_deadlock, 
                                int node_count, const char* cycle_path, const char* edge_type) {
    snprintf(buffer, size,
        "{\"has_deadlock\":%s,\"node_count\":%d,"
        "\"cycle_path\":\"%s\",\"edge_type\":\"%s\"}",
        has_deadlock ? "true" : "false",
        node_count,
        cycle_path ? cycle_path : "",
        edge_type ? edge_type : "");
}

static void perf_metrics_to_json(char* buffer, size_t size, long lock_time_ns, int failed_attempts) {
    snprintf(buffer, size,
        "{\"lock_time_ns\":%ld,\"failed_attempts\":%d}",
        lock_time_ns, failed_attempts);
}

// Main logging function with struct support
int log_train_event_csv_ex(FILE* file, 
                          const char* component,
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
                          int failed_attempts) {
    
    if (!file) return -1;

    // Get timestamp
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    // Format timestamp with nanosecond precision
    char precise_time[32];
    struct tm *tm_info = localtime(&ts.tv_sec);
    strftime(precise_time, sizeof(precise_time), "%Y-%m-%d %H:%M:%S", tm_info);
    char full_timestamp[48];
    snprintf(full_timestamp, sizeof(full_timestamp), "%s.%09ld", precise_time, ts.tv_nsec);

    // Generate JSON strings for complex data
    char resource_json[512];
    char train_json[1024];
    char deadlock_json[256];
    char perf_json[128];

    shared_intersection_to_json(resource_json, sizeof(resource_json), resource_state);
    train_entry_to_json(train_json, sizeof(train_json), train_state, current_position);
    deadlock_info_to_json(deadlock_json, sizeof(deadlock_json), 
                         has_deadlock, node_count, cycle_path, edge_type);
    perf_metrics_to_json(perf_json, sizeof(perf_json), 
                        lock_time_ns, failed_attempts);

    // Write directly to file
    fprintf(file, "%s,%s,%s,%s,%d,%s,%s,%s,%d,%s,%s,%s,%s,%s\n",
            full_timestamp,
            __FILE__,
            __func__,
            component ? component : "",
            train_id,
            intersection_id ? intersection_id : "",
            action ? action : "",
            status ? status : "",
            pid,
            error_msg ? error_msg : "",
            resource_json,
            deadlock_json,
            train_json,
            perf_json);
    
    fflush(file);
    return 0;
}

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

    // Write CSV header with fixed columns and JSON columns
    fprintf(file, "sys_time,calling_file,calling_function,"
                 "component,train_id,intersection_id,action,status,pid,error_msg,"
                 "resource_state,deadlock_info,train_state,perf_metrics\n");
    fflush(file);

    return file;
}

int log_train_event_csv(FILE* file, const char* csv_data, const char* calling_file, const char* calling_func) {
    if (!file) {
        return -1;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    // Format timestamp with nanosecond precision
    char precise_time[32];
    struct tm *tm_info = localtime(&ts.tv_sec);
    strftime(precise_time, sizeof(precise_time), "%Y-%m-%d %H:%M:%S", tm_info);
    char full_timestamp[48];
    snprintf(full_timestamp, sizeof(full_timestamp), "%s.%09ld", precise_time, ts.tv_nsec);

    // Write CSV line with timestamp, caller info, and provided data
    fprintf(file, "%s,%s,%s,%s\n", 
            full_timestamp,
            calling_file ? calling_file : "",
            calling_func ? calling_func : "",
            csv_data ? csv_data : "");
    fflush(file);
    
    return 0;
}

void csv_logger_close(FILE* file) {
    if (file) {
        fclose(file);
    }
}