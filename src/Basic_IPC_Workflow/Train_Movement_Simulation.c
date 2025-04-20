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

#include "logger.h" // log_init, LOG_CLIENT, log_close
#include "parser.h" // getTrains, TrainEntry
#include "resource_allocation_graph.h"

#define MSG_KEY 1234
#define MAX_NAME 64

// IPC message format
typedef struct
{
    long mtype; // 1 for requests
    int train_id;
    char intersection[MAX_NAME];
    char action[8]; // "ACQUIRE", "RELEASE"
} Message;

// trains process: ACQUIRE then WAIT then GRANT then TRAVEL then ACQUIRE NEXT then RELEASE OLD then WAIT OK then repeat
void run_train(int msgid, int train_id, char *route[], int route_len)
{
    Message req, resp;

    // request each intersection then wait for GRANT
    for (int i = 0; i < route_len; i++)
    {
        // send ACQUIRE for route[i]. this is so we can get the trains entire route before traveling
        req.mtype = 1;
        req.train_id = train_id;
        strncpy(req.intersection, route[i], MAX_NAME - 1);
        req.intersection[MAX_NAME - 1] = '\0';
        snprintf(req.action, sizeof(req.action), "ACQUIRE");
        if (msgsnd(msgid, &req, sizeof(req) - sizeof(long), 0) == -1)
        {
            LOG_TRAIN(train_id, "msgsnd(ACQUIRE) failed: %s", strerror(errno));
            exit(1);
        }
        LOG_TRAIN(train_id, "Sent ACQUIRE request for %s", route[i]);

        // wait for GRANT for this intersection
        do
        {
            if (msgrcv(msgid, &resp, sizeof(resp) - sizeof(long),
                       train_id + 100, 0) == -1)
            {
                LOG_TRAIN(train_id, "msgrcv(GRANT) failed: %s", strerror(errno));
                exit(1);
            }
        } while (strcmp(resp.action, "GRANT") != 0);
        //LOG_TRAIN(train_id, "Received GRANT for %s", resp.intersection);
    }

    // now that all intersections are held we can traverse them
    for (int i = 0; i < route_len; i++)
    {
        sleep(1);
        LOG_TRAIN(train_id, "Traversed %s", route[i]);
    }
    

    // release each intersection
    for (int i = 0; i < route_len; i++)
    {
        // send RELEASE for route[i] NOTE: Might want to change this to release all at same time
        req.mtype = 1;
        req.train_id = train_id;
        strncpy(req.intersection, route[i], MAX_NAME - 1);
        req.intersection[MAX_NAME - 1] = '\0';
        snprintf(req.action, sizeof(req.action), "RELEASE");
        if (msgsnd(msgid, &req, sizeof(req) - sizeof(long), 0) == -1)
        {
            LOG_TRAIN(train_id, "msgsnd(RELEASE) failed: %s", strerror(errno));
            exit(1);
        }
        //LOG_TRAIN(train_id, "Sent RELEASE for %s", route[i]);
    }

    // wait for OK on each
    for (int i = 0; i < route_len; i++)
    {
        do
        {
            if (msgrcv(msgid, &resp, sizeof(resp) - sizeof(long),
                       train_id + 100, 0) == -1)
            {
                LOG_TRAIN(train_id, "msgrcv(OK) failed: %s", strerror(errno));
                exit(1);
            }
        } while (strcmp(resp.action, "OK") != 0);
        //LOG_TRAIN(train_id, "Received OK for %s", resp.intersection);
    }
}

int main()
{
    // init logging
    log_init("simulation.log", 0);
    //LOG_SERVER("Starting train simulator");

    // connect to the message queue
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0)
    {
        LOG_SERVER("msgget failed: %s", strerror(errno));
        exit(1);
    }
    //LOG_SERVER("Message queue ready (ID: %d)", msgid);

    // parse trains.txt
    TrainEntry trains[ITEM_COUNT_MAX];
    int train_count = getTrains(trains);
    if (train_count < 0)
    {
        LOG_SERVER("Failed to parse trains.txt");
        exit(1);
    }
    LOG_SERVER("Parsed %d trains\n", train_count);

    // fork one child per train
    pid_t pids[ITEM_COUNT_MAX];
    for (int i = 0; i < train_count; i++)
    {
        // build the route pointer array
        int len = trains[i].routeLength;
        char *routePtrs[ITEM_COUNT_MAX];
        for (int j = 0; j < len; j++)
            routePtrs[j] = trains[i].route[j];

        // extract numeric ID from "TrainX"
        int train_id = atoi(trains[i].id + 5);

        pid_t pid = fork();
        if (pid < 0)
        {
            LOG_SERVER("fork failed: %s", strerror(errno));
            exit(1);
        }
        if (pid == 0)
        {
            // child: run its train
            run_train(msgid, train_id, routePtrs, len);
            exit(0);
        }
        // parent: record child's PID
        pids[i] = pid;
    }

// Initialize the Resource Allocation Graph
init_graph();

Message msg; // Message structure for IPC
int stop_flag = 0; // Flag to stop the server loop

while (!stop_flag) {
    // Wait for a message from any train (mtype = 1)
    if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 1, 0) == -1) {
        LOG_SERVER("msgrcv failed: %s", strerror(errno));
        exit(1);
    }

    // Handle STOP message to terminate the loop
    if (strcmp(msg.action, "STOP") == 0) {
        LOG_SERVER("Received STOP signal.");
        stop_flag = 1; // Set the flag to stop the server loop
        continue; // Skip processing other messages
    }

    LOG_SERVER("Received %s from Train %d for %s", msg.action, msg.train_id, msg.intersection);

    if (strcmp(msg.action, "ACQUIRE") == 0) { // Handle ACQUIRE request
        // Check if the intersection is available
        // (In a real implementation, you would check the intersection's state here)
        // Add request edge to RAG (Train ➝ Intersection)
        add_request_edge(msg.train_id, msg.intersection);

        // Check for deadlock
        if (detect_deadlock()) { // If deadlock is detected
            LOG_SERVER("DEADLOCK DETECTED: Train %d waiting for %s", msg.train_id, msg.intersection);
            print_graph();  // Show the current RAG state
            continue; // Skip granting to avoid worsening the deadlock
        }

        // Grant access and convert edge to allocation (Intersection -> Train)
        add_allocation_edge(msg.train_id, msg.intersection);

        // Send GRANT message to train
        Message grant = {.mtype = msg.train_id + 100, .train_id = msg.train_id}; // mtype for GRANT is train_id + 100
        strncpy(grant.intersection, msg.intersection, MAX_NAME);
        snprintf(grant.action, sizeof(grant.action), "GRANT"); // Set action to GRANT
        msgsnd(msgid, &grant, sizeof(grant) - sizeof(long), 0); // Send the message
        LOG_SERVER("Granted %s to Train %d", msg.intersection, msg.train_id);
    }

    else if (strcmp(msg.action, "RELEASE") == 0) { // Handle RELEASE request
        // Remove allocation edge (Intersection -> Train)
        remove_edges(msg.train_id, msg.intersection); // Remove the allocation edge

        // Send OK message to train
        Message ok = {.mtype = msg.train_id + 100, .train_id = msg.train_id}; 
        strncpy(ok.intersection, msg.intersection, MAX_NAME);
        snprintf(ok.action, sizeof(ok.action), "OK"); // Set action to OK
        msgsnd(msgid, &ok, sizeof(ok) - sizeof(long), 0); // Send the message
        LOG_SERVER("Released %s from Train %d", msg.intersection, msg.train_id);
    }
}

    // wait for all train children to finish
    for (int i = 0; i < train_count; i++)
    {
        waitpid(pids[i], NULL, 0);
    }
    LOG_SERVER("All %d trains have finished", train_count);

    // tell the server to stop
    Message stop = {.mtype = 1, .train_id = 0};
    memset(stop.intersection, 0, sizeof(stop.intersection));
    snprintf(stop.action, sizeof(stop.action), "STOP");
    if (msgsnd(msgid, &stop, sizeof(stop) - sizeof(long), 0) == -1)
    {
        LOG_SERVER("Failed to send STOP: %s", strerror(errno));
    }
    else
    {
        LOG_SERVER("Sent STOP to server");
    }

    // exit
    LOG_SERVER("Train simulator exiting");
    log_close();
    return 0;
}