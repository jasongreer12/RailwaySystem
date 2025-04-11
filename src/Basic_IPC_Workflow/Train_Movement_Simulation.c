// Train_Movement_Simulation.c
// Author: Zachary Oyer
// Date: 4-4-2025
// Simulates train processes requesting and releasing intersections using
// IPC. Forks multiple train processes and runs a central server loop to
// receive and display IPC messages.
/*#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>

#define MSG_KEY 1234
#define MAX_NAME 64

typedef struct {
    long mtype;
    int train_id;
    char intersection[MAX_NAME];
    char action[8]; // "ACQUIRE" or "RELEASE"
} Message;

void send_msg(int msgid, int train_id, const char* intersection, const char* action) {
    Message msg;
    msg.mtype = 1;
    msg.train_id = train_id;
    strncpy(msg.intersection, intersection, MAX_NAME);
    strncpy(msg.action, action, sizeof(msg.action));
    msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
}

void run_train(int msgid, int train_id, const char* route[], int route_len) {
    for (int i = 0; i < route_len; i++) {
        send_msg(msgid, train_id, route[i], "ACQUIRE");
        printf("Train %d requesting %s\n", train_id, route[i]);
        sleep(1); // simulate waiting

        printf("Train %d moving through %s\n", train_id, route[i]);
        sleep(1); // simulate movement

        send_msg(msgid, train_id, route[i], "RELEASE");
        printf("Train %d released %s\n", train_id, route[i]);
    }
}

void server_loop(int msgid) {
    Message msg;
    while (1) {
        if (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), 0, 0) > 0) {
            printf("[SERVER] %s from Train %d for %s\n", msg.action, msg.train_id, msg.intersection);
        }
    }
}

int main() {
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);

    const char* route1[] = {"IntersectionA", "IntersectionB", "IntersectionC"};
    const char* route2[] = {"IntersectionB", "IntersectionD", "IntersectionE"};

    pid_t pid1 = fork();
    if (pid1 == 0) {
        run_train(msgid, 1, route1, 3);
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        run_train(msgid, 2, route2, 3);
        exit(0);
    }

    server_loop(msgid);  // run forever for now
    
    // This cleanup code won't be reached in this example,
    // but good practice to include it
    for (int i = 0; i < num_intersections; i++) {
        cleanup_locks(&intersections[i]);
    }
    
    return 0;
}
*/
// Train_Movement_Simulation.c
// Author: Zachary Oyer 
// Date: 4-4-2025
// Dynamically reads intersections.txt and trains.txt, initializes locks,
// forks train processes, and runs a server loop to handle ACQUIRE/RELEASE IPC.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>

#include "logger.h"              // Updated logger interface
#include "parser.h"              // from parser/parser.h
#include "intersection_locks.h"  // your lock primitives

#define MSG_KEY 1234
#define MAX_NAME 64

// --- Shared state ---
Intersection intersections[ITEM_COUNT_MAX];
int num_intersections = 0;

// --- IPC message format ---
typedef struct {
    long mtype;                 // required by System V message queues
    int train_id;
    char intersection[MAX_NAME];
    char action[8];             // "ACQUIRE" or "RELEASE"
} Message;

// Find an intersection by name
int find_intersection(const char* name) {
    for (int i = 0; i < num_intersections; i++) {
        if (strcmp(intersections[i].name, name) == 0) {
            return i;
        }
    }
    return -1; // Not found
}

// --- Initialize intersections from parser data ---
void setup_intersections() {
    IntersectionEntry iEntries[ITEM_COUNT_MAX];
    num_intersections = getIntersections(iEntries);
    LOG_SERVER("Parsed %d intersections", num_intersections);
    
    for (int i = 0; i < num_intersections; i++) {
        strncpy(intersections[i].name, iEntries[i].id, MAX_NAME);
        intersections[i].capacity = iEntries[i].capacity;
        if (iEntries[i].capacity == 1)
            init_mutex_lock(&intersections[i]);
        else
            init_semaphore_lock(&intersections[i]);
    }
}

// --- Send a message on the queue ---
void send_msg(int msgid, int train_id, const char* intersection, const char* action) {
    Message msg;
    msg.mtype = 1;
    msg.train_id = train_id;
    strncpy(msg.intersection, intersection, MAX_NAME);
    strncpy(msg.action, action, sizeof(msg.action));
    if (msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
        // Log the error using the SERVER logger
        LOG_SERVER("Failed to send message: %s", strerror(errno));
    }
}

// --- Train process: follow its route, sending ACQUIRE/RELEASE ---
void run_train(int msgid, int train_id, char* route[], int route_len) {
    for (int i = 0; i < route_len; i++) {
        send_msg(msgid, train_id, route[i], "ACQUIRE");
        // Use the TRAIN logging macro for train-specific messages
        LOG_TRAIN(train_id, "Sent ACQUIRE request for %s", route[i]);
        sleep(1);

        LOG_TRAIN(train_id, "Moving through %s", route[i]);
        sleep(1);

        send_msg(msgid, train_id, route[i], "RELEASE");
        LOG_TRAIN(train_id, "Released %s", route[i]);
    }
}

// --- Central server loop: handle ACQUIRE/RELEASE requests ---
void server_loop(int msgid) {
    Message msg;
    while (1) {
        if (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), 0, 0) > 0) {
            LOG_SERVER("%s from Train %d for %s", msg.action, msg.train_id, msg.intersection);

            int idx = find_intersection(msg.intersection);
            if (idx < 0) {
                LOG_SERVER("Unknown intersection %s", msg.intersection);
                continue;
            }

            if (strcmp(msg.action, "ACQUIRE") == 0) {
                if (acquire_lock(&intersections[idx]) == 0)
                    LOG_SERVER("Granted %s to Train %d", msg.intersection, msg.train_id);
                else
                    LOG_SERVER("Denied %s to Train %d", msg.intersection, msg.train_id);
            } else {  // RELEASE
                if (release_lock(&intersections[idx]) == 0)
                    LOG_SERVER("Released %s from Train %d", msg.intersection, msg.train_id);
                else
                    LOG_SERVER("Failed to release %s from Train %d", msg.intersection, msg.train_id);
            }
        } else {
            LOG_SERVER("msgrcv failed: %s", strerror(errno));
        }
    }
}

int main() {
    log_init("simulation.log");
    LOG_SERVER("Initializing Train Movement Simulation");
    
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0) {
        LOG_SERVER("msgget failed: %s", strerror(errno));
        exit(1);
    }

    setup_intersections();
    
    TrainEntry tEntries[ITEM_COUNT_MAX];
    int train_count = getTrains(tEntries);
    LOG_SERVER("Parsed %d trains", train_count);
    
    for (int i = 0; i < train_count; i++) {
        char *routePtrs[ITEM_COUNT_MAX];
        int len = tEntries[i].routeLength;
        for (int j = 0; j < len; j++)
            routePtrs[j] = tEntries[i].route[j];
        
        // Extract train id from the id string (assumes the id string contains "Train" as a prefix)
        int train_id = atoi(tEntries[i].id + 5);
        
        pid_t pid = fork();
        if (pid < 0) {
            LOG_SERVER("fork failed: %s", strerror(errno));
            exit(1);
        }
        if (pid == 0) {
            run_train(msgid, train_id, routePtrs, len);
            exit(0);
        }
    }
    
    server_loop(msgid);
    return 0;
}