// This is the old code used in Train_Movement_Simulation.c
// Author: Zachary Oyer
// Date: 4-4-2025
// Group B
// Simulates train processes requesting and releasing intersections using
// System V message queues for IPC. Forks multiple train processes and
// runs a central server loop to handle intersection locks.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include "intersection_locks.h"  // Custom locking functions (mutex/semaphore)
#include "ipc.h"             // Message struct and send_message function
//#include "../logger/csv_logger.h" // For logging

/*
See csv_logger.h for definitions.
Update relevant fields with local variables to pass into csv log for debugging.
LOG_CSV(0, "SYSTEM", "INIT_INTERSECTION", "SUCCESS", getpid(), NULL, NULL, NULL, 0, false, 0, NULL, NULL);
*/

#define MSG_KEY 1234
#define MAX_NAME 64
#define MAX_INTERSECTIONS 10

// Global array to store intersection definitions and lock info
Intersection intersections[MAX_INTERSECTIONS];
int num_intersections = 0;

// IPC message structure
// typedef struct {
//     long mtype;                     // Required for System V message queues
//     int train_id;                   // ID of the train sending the message
//     char intersection[MAX_NAME];   // Intersection name
//     char action[8];                // "ACQUIRE" or "RELEASE"
// } Message;
// Jarett: commenting out because it is a duplicate of message struct in ipc.h

// Sends an IPC message to the message queue
void send_msg(int msgid, int train_id, const char* intersection, const char* action) {
    Message msg;
    msg.mtype = 1;
    msg.train_id = train_id;
    strncpy(msg.intersection, intersection, MAX_NAME);
    strncpy(msg.action, action, sizeof(msg.action));
    msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
}

// Simulates a train traveling through a list of intersections
void run_train(int msgid, int train_id, const char* route[], int route_len) {
    for (int i = 0; i < route_len; i++) {
        // Request to enter intersection
        send_msg(msgid, train_id, route[i], "ACQUIRE");
        printf("Train %d requesting %s\n", train_id, route[i]);
        sleep(1); // Simulate wait for lock

        // Train moves through the intersection
        printf("Train %d moving through %s\n", train_id, route[i]);
        sleep(1); // Simulate movement time

        // Release the intersection
        send_msg(msgid, train_id, route[i], "RELEASE");
        printf("Train %d released %s\n", train_id, route[i]);
    }
}

// Initializes all intersections with their names, capacities, and locks
void init_intersections() {
    // IntersectionA - single train at a time (mutex)
    strcpy(intersections[0].name, "IntersectionA");
    intersections[0].capacity = 1;
    init_mutex_lock(&intersections[0]);
    
    // IntersectionB - allows 2 trains (semaphore)
    strcpy(intersections[1].name, "IntersectionB");
    intersections[1].capacity = 2;
    init_semaphore_lock(&intersections[1]);
    
    // IntersectionC - single train (mutex)
    strcpy(intersections[2].name, "IntersectionC");
    intersections[2].capacity = 1;
    init_mutex_lock(&intersections[2]);
    
    // IntersectionD - allows 3 trains (semaphore)
    strcpy(intersections[3].name, "IntersectionD");
    intersections[3].capacity = 3;
    init_semaphore_lock(&intersections[3]);
    
    // IntersectionE - single train (mutex)
    strcpy(intersections[4].name, "IntersectionE");
    intersections[4].capacity = 1;
    init_mutex_lock(&intersections[4]);
    
    num_intersections = 5;
    printf("Initialized %d intersections\n", num_intersections);
}

// Searches the intersections array for a given name
int find_intersection(const char* name) {
    for (int i = 0; i < num_intersections; i++) {
        if (strcmp(intersections[i].name, name) == 0) {
            return i;
        }
    }
    return -1; // Not found
}

// Central server loop to process IPC messages and handle locks
void server_loop(int msgid) {
    Message msg;

    while (1) {
        // Receive next message from queue
        if (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), 0, 0) > 0) {
            printf("[SERVER] %s from Train %d for %s\n", msg.action, msg.train_id, msg.intersection);
            
            int idx = find_intersection(msg.intersection);
            if (idx == -1) {
                printf("[SERVER] Intersection %s not found\n", msg.intersection);
                continue;
            }
            /*
            if (strcmp(msg.action, "ACQUIRE") == 0) {
                // Attempt to acquire the lock
                if (acquire_lock(&intersections[idx], msg.train_id) == 0) {
                    printf("[SERVER] Granted %s to Train %d\n", msg.intersection, msg.train_id);
                } else {
                    printf("[SERVER] Could not grant %s to Train %d\n", msg.intersection, msg.train_id);
                }
            } else if (strcmp(msg.action, "RELEASE") == 0) {
                // Attempt to release the lock
                if (release_lock(&intersections[idx], msg.train_id) == 0) {
                    printf("[SERVER] Released %s from Train %d\n", msg.intersection, msg.train_id);
                } else {
                    printf("[SERVER] Failed to release %s from Train %d\n", msg.intersection, msg.train_id);
                }
            }*/
        }
    }
}

int main() {
    // Create the System V message queue
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);

    // Set up all intersections and their lock types
    init_intersections();

    // Define the travel routes for the trains
    const char* route1[] = {"IntersectionA", "IntersectionB", "IntersectionC"};
    const char* route2[] = {"IntersectionB", "IntersectionD", "IntersectionE"};

    // Fork Train 1
    pid_t pid1 = fork();
    if (pid1 == 0) {
        run_train(msgid, 1, route1, 3);
        exit(0);
    }

    // Fork Train 2
    pid_t pid2 = fork();
    if (pid2 == 0) {
        run_train(msgid, 2, route2, 3);
        exit(0);
    }

    // Run the central server loop
    server_loop(msgid);  // NOTE: this loop runs forever in current version

    // Cleanup (unreachable, but good practice if you add exit conditions)
    for (int i = 0; i < num_intersections; i++) {
        cleanup_locks(&intersections[i]);
    }

    return 0;
}