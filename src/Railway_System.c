/*
Author: Jarett Woodard
Group: B
Email: jarett.woodard@okstate.edu
Date: 4.4.2025
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>

#include "logger/logger.h"                         // Jason Greer
#include "Basic_IPC_Workflow/ipc.h"                // Zach Oyer
#include "parser/parser.h"                         // Jarett Woodard
#include "Basic_IPC_Workflow/intersection_locks.h" // Jake Pinell
#include "Shared_Memory_Setup/Memory_Segments.h"   // Steve Kuria

// This file uses code from server.c authored by Jason Greer

#define LINE_MAX 256

// Helper to map intersection name → index in iEntries[]
static int find_intersection_index(const IntersectionEntry entries[],
                                   int count,
                                   const char *name)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(entries[i].id, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

int main()
{
    log_init("simulation.log", 1);
    LOG_SERVER("Initializing Train Movement Simulation");

    // set up shared memory (for whatever data you need there)
    size_t shm_size;
    SharedIntersection *shared_intersections =
        init_shared_memory("/intersection_shm", &shm_size);
    if (!shared_intersections)
    {
        LOG_SERVER("Failed to initialize shared memory");
        fprintf(stderr, "[SERVER] Failed to initialize shared memory.\n");
        exit(1);
    }
    LOG_SERVER("Shared memory initialized");

    // parse trains
    TrainEntry trains[LINE_MAX];
    int trainCount = getTrains(trains);
    LOG_SERVER("Parsed %d trains", trainCount);
    printTrainEntries(trains, trainCount);

    // parse intersections
    IntersectionEntry iEntries[LINE_MAX];
    int intersectionCount = getIntersections(iEntries);
    LOG_SERVER("Parsed %d intersections", intersectionCount);
    printIntersectionEntries(iEntries, intersectionCount);

    // build and initialize local locks array
    Intersection locks[LINE_MAX];
    for (int i = 0; i < intersectionCount; i++)
    {
        // Copy name & capacity
        strncpy(locks[i].name, iEntries[i].id, MAX_NAME_LENGTH - 1);
        locks[i].name[MAX_NAME_LENGTH - 1] = '\0';
        locks[i].capacity = iEntries[i].capacity;

        // Initialize the correct lock
        if (locks[i].capacity == 1)
        {
            init_mutex_lock(&locks[i]);
            LOG_SERVER("Initialized mutex for %s (capacity=1)", locks[i].name);
        }
        else
        {
            init_semaphore_lock(&locks[i]);
            LOG_SERVER("Initialized semaphore for %s (capacity=%d)",
                       locks[i].name, locks[i].capacity);
        }
    }

    // set up message queue
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0)
    {
        LOG_SERVER("msgget failed: %s", strerror(errno));
        perror("[SERVER] msgget");
        exit(1);
    }
    LOG_SERVER("Message queue ready (ID: %d)", msgid);
    printf("[SERVER] Message queue ready (ID: %d)\n", msgid);

    // main server loop
    Message req, resp;
    while (1)
    {
        if (msgrcv(msgid, &req, sizeof(req) - sizeof(long), 1, 0) == -1)
        {
            LOG_SERVER("msgrcv failed: %s", strerror(errno));
            perror("[SERVER] msgrcv");
            continue;
        }

        // STOP? then break
        if (strcmp(req.action, "STOP") == 0)
        {
            LOG_SERVER("Received STOP signal. Exiting server loop");
            break;
        }

        LOG_SERVER("Received: Train %d requests \"%s\" on %s",
                   req.train_id, req.action, req.intersection);

        // Prepare common parts of response
        memset(&resp, 0, sizeof(resp));
        resp.mtype = req.train_id + 100;
        resp.train_id = req.train_id;
        strncpy(resp.intersection, req.intersection, sizeof(resp.intersection) - 1);

        // Lookup which lock to use
        int idx = find_intersection_index(iEntries, intersectionCount, req.intersection);
        if (idx < 0)
        {
            strncpy(resp.action, "FAIL", sizeof(resp.action) - 1);
            LOG_SERVER("Unknown intersection %s", req.intersection);
        }
        else
        {
            // ACQUIRE or RELEASE on locks[idx]
            if (strcmp(req.action, "ACQUIRE") == 0)
            {
                if (acquire_lock(&locks[idx]) == 0)
                {
                    strncpy(resp.action, "GRANT", sizeof(resp.action) - 1);
                    LOG_SERVER("GRANTED %s to Train %d",
                               req.intersection, req.train_id);
                }
                else
                {
                    strncpy(resp.action, "DENY", sizeof(resp.action) - 1);
                    LOG_SERVER("DENIED %s to Train %d",
                               req.intersection, req.train_id);
                }
            }
            else
            { // RELEASE
                if (release_lock(&locks[idx]) == 0)
                {
                    strncpy(resp.action, "OK", sizeof(resp.action) - 1);
                    LOG_SERVER("Released %s from Train %d",
                               req.intersection, req.train_id);
                }
                else
                {
                    strncpy(resp.action, "FAIL", sizeof(resp.action) - 1);
                    LOG_SERVER("Failed to release %s from Train %d",
                               req.intersection, req.train_id);
                }
            }
        }

        // Send the response
        if (msgsnd(msgid, &resp, sizeof(resp) - sizeof(long), 0) == -1)
        {
            LOG_SERVER("msgsnd failed: %s", strerror(errno));
            perror("[SERVER] msgsnd");
        }
        else
        {
            LOG_SERVER("Sent response: Train %d \"%s\" on %s",
                       resp.train_id, resp.action, resp.intersection);
            printf("[SERVER] Sent response: Train %d \"%s\" on %s\n",
                   resp.train_id, resp.action, resp.intersection);
        }
    }

    // clean the queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1)
    {
        LOG_SERVER("msgctl(IPC_RMID) failed: %s", strerror(errno));
        perror("[SERVER] msgctl");
    }
    else
    {
        LOG_SERVER("Message queue removed");
        printf("[SERVER] Message queue removed. Exiting.\n");
    }

    // cleanup shared memory
    destroy_shared_memory(shared_intersections, "/intersection_shm", shm_size);
    LOG_SERVER("Shared memory cleaned up");

    // final log & exit
    LOG_SERVER("SIMULATION COMPLETE. All trains reached destinations.");
    log_close();
    return 0;
}