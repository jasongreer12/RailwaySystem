// Train_Movement_Simulation.c
// Author: Jason Greer
// Date: 4-18-2025
// Group B
// Simulates train processes requesting and releasing intersections using
// System V message queues for IPC. Forks multiple train processes and
// runs a central server loop to handle intersection locks.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "logger.h"       // log_init, LOG_CLIENT, log_close
#include "parser.h"       // getTrains, TrainEntry
#include "resource_allocation_graph.h"
#include "../Shared_Memory_Setup/Memory_Segments.h" // SharedIntersection

#define MSG_KEY    1234
#define MAX_NAME   64

// IPC message format 
typedef struct {
    long mtype;               // 1 for requests
    int  train_id;
    char intersection[MAX_NAME];
    char action[8];           // "ACQUIRE", "RELEASE"
} Message;

// each trains workflow: ACQUIRE then WAIT then GRANT then TRAVEL then RELEASE then WAIT OK
void run_train(int msgid, int train_id, char *route[], int route_len) {
    //moved generation of comp string to macro in logger.h
    Message req, resp;
    for (int i = 0; i < route_len; i++) {
        // send ACQUIRE
        req.mtype      = 1;
        req.train_id   = train_id;
        strncpy(req.intersection, route[i], MAX_NAME-1);
        req.intersection[MAX_NAME-1] = '\0';
        snprintf(req.action, sizeof(req.action), "ACQUIRE");
        
        if (msgsnd(msgid, &req, sizeof(req)-sizeof(long), 0) == -1) {
            LOG_TRAIN(train_id, "msgsnd(ACQUIRE) failed: %s", strerror(errno));
            exit(1);
        }
        LOG_TRAIN(train_id, "Sent ACQUIRE request for %s", route[i]);

        // wait only for grant
        do {
            if (msgrcv(msgid, &resp, sizeof(resp)-sizeof(long),
                       train_id+100, 0) == -1) {
                LOG_TRAIN(train_id, "msgrcv(GRANT) failed: %s", strerror(errno));
                exit(1);
            }
            LOG_TRAIN(train_id, "Received %s for %s",
                      resp.action, resp.intersection);
        } while (strcmp(resp.action, "GRANT") != 0);

        // simulate traversal
        sleep(1);

        // send RELEASE
        snprintf(req.action, sizeof(req.action), "RELEASE");
        if (msgsnd(msgid, &req, sizeof(req)-sizeof(long), 0) == -1) {
            LOG_TRAIN(train_id, "msgsnd(RELEASE) failed: %s", strerror(errno));
            exit(1);
        }
        LOG_TRAIN(train_id, "Sent RELEASE for %s", route[i]);

        // wait for OK 
        do {
            if (msgrcv(msgid, &resp, sizeof(resp)-sizeof(long),
                       train_id+100, 0) == -1) {
                LOG_TRAIN(train_id, "msgrcv(OK) failed: %s", strerror(errno));
                exit(1);
            }
            LOG_TRAIN(train_id, "Received %s for %s",
                      resp.action, resp.intersection);
        } while (strcmp(resp.action, "OK") != 0);
    }
}

int main() {
    // init logging
    log_init("simulation.log", 0);
    
    //verify shared memory is available. initialize if not available
    if (!shared_intersections) {
        size_t shm_size;
        shared_intersections = init_shared_memory("/intersection_shm", &shm_size);
        if (!shared_intersections) {
            fprintf(stderr, "Failed to connect to shared memory\n");
            exit(1);
        }
    }
    
    LOG_SERVER("Starting train simulator");

    // connect to the message queue
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0) {
        LOG_SERVER("msgget failed: %s", strerror(errno));
        exit(1);
    }
    LOG_SERVER("Message queue ready (ID: %d)", msgid);

    // parse trains.txt
    TrainEntry trains[ITEM_COUNT_MAX];
    int train_count = getTrains(trains);
    if (train_count < 0) {
        LOG_SERVER("Failed to parse trains.txt");
        exit(1);
    }
    LOG_SERVER("Parsed %d trains", train_count);

    // fork one child per train
    pid_t pids[ITEM_COUNT_MAX];
    for (int i = 0; i < train_count; i++) {
        // build the route pointer array
        int len = trains[i].routeLength;
        char *routePtrs[ITEM_COUNT_MAX];
        for (int j = 0; j < len; j++)
            routePtrs[j] = trains[i].route[j];

        // extract numeric ID from "TrainX"
        int train_id = atoi(trains[i].id + 5);

        pid_t pid = fork();
        if (pid < 0) {
            LOG_SERVER("fork failed: %s", strerror(errno));
            exit(1);
        }
        if (pid == 0) {
            // child: run its train
            run_train(msgid, train_id, routePtrs, len);
            exit(0);
        }
        // parent: record child's PID
        pids[i] = pid;
    }

    // wait for all train children to finish
    int remaining_trains = train_count;
    while (remaining_trains > 0) { //force wait without forcing order
        int status;
        pid_t finished_pid = waitpid(-1, &status, 0);  // Wait for any child to finish
        
        for (int i = 0; i < train_count; i++) {
            if (pids[i] == finished_pid) {
                if (WIFEXITED(status)) {//if exited, log exit status
                    LOG_SERVER("Train %d exited with status %d", i+1, WEXITSTATUS(status));
                }
                break;
            }
        }
        remaining_trains--; //decrement remaining trains
    }
    LOG_SERVER("All %d trains have finished", train_count);

    // tell the Railway System to stop and wait for acknowledgment
    Message stop = { .mtype = 1, .train_id = 0 };
    memset(stop.intersection, 0, sizeof(stop.intersection));
    snprintf(stop.action, sizeof(stop.action), "STOP");
    
    if (msgsnd(msgid, &stop, sizeof(stop) - sizeof(long), 0) == -1) {
        LOG_SERVER("Failed to send STOP: %s", strerror(errno));
    } else {
        LOG_SERVER("Sent STOP to Railway System");
        //log kept printing after complete because of concurrent processes
        //adding pause to allow other processes to finish.
        sleep(1);
    }

    //unmap the memory to prevent memory leaks
    munmap(shared_intersections, sizeof(SharedIntersection) * NUM_INTERSECTIONS);
    shared_intersections = NULL;

    // exit
    LOG_SERVER("Train simulator exiting");
    log_close();
    return 0;
}