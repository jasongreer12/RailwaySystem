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

<<<<<<< Updated upstream
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
=======
// Helper function: Given an intersection name, find its index from the parsed intersections.
// Assumes IntersectionEntry has a 'name' field.
int getIntersectionIndex(const char *name, IntersectionEntry *entries, int count) {
    for (int i = 0; i < count; i++) {
        if (strncmp(name, entries[i].id, MAX_NAME) == 0) {
            return i;
        }
    }
    return -1;  // not found
}


// Helper function: Given an intersection name, find its index from the parsed intersections.
// Assumes IntersectionEntry has a 'name' field.
int getIntersectionIndex(const char *name, IntersectionEntry *entries, int count) {
    for (int i = 0; i < count; i++) {
        if (strncmp(name, entries[i].id, MAX_NAME) == 0) {
            return i;
        }
    }
    return -1;  // not found
}


int main(){
    // Initialize shared memory for intersections
    size_t shm_size;
    SharedIntersection *shared_intersections = init_shared_memory("/intersection_shm", &shm_size);
    if (!shared_intersections) {
        fprintf(stderr, "[SERVER] Failed to initialize shared memory.\n");
        return 1;
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
    // build and initialize local locks array
    Intersection locks[LINE_MAX];
    for (int i = 0; i < intersectionCount; i++)
    {
        // Copy name & capacity
        strncpy(locks[i].name, iEntries[i].id, MAX_NAME_LENGTH - 1);
        locks[i].name[MAX_NAME_LENGTH - 1] = '\0';
        locks[i].capacity = iEntries[i].capacity;
=======
     // create (or open) the message queue using the defined MSG_KEY
     int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
     if (msgid < 0) {
         perror("[SERVER] msgget");
         exit(EXIT_FAILURE);
     }
     printf("[SERVER] Message queue ready (ID: %d)\n", msgid);
 
     Message rcv_msg, snd_msg;
     while (1) {
         // wait for any message of type 1 (ACQUIRE/RELEASE requests) as all messages have this type for right now
         if (msgrcv(msgid, &rcv_msg, sizeof(Message) - sizeof(long), 1, 0) == -1) {
             perror("[SERVER] msgrcv");
             continue;
         }
         
         printf("[SERVER] Received: Train %d requests \"%s\" on %s\n",
                rcv_msg.train_id, rcv_msg.action, rcv_msg.intersection);
 
         // check for a stop condition 
         if (strncmp(rcv_msg.action, "STOP", 4) == 0) {
             printf("[SERVER] Received STOP signal. Shutting down.\n");
             break;
         }
         
         // prepare the response message
         // set the responses mtype to the trains id so the train can receive it
         memset(&snd_msg, 0, sizeof(snd_msg));
         snd_msg.mtype = rcv_msg.train_id + 100;  // response addressed to the requesting train
         snd_msg.train_id = rcv_msg.train_id;
         strncpy(snd_msg.intersection, rcv_msg.intersection, MAX_NAME);
         
        // Get the intersection index from the parsed entries
        int idx = getIntersectionIndex(rcv_msg.intersection, intersections, intersectionCount);

        if (idx < 0) {
            fprintf(stderr, "[SERVER] Unknown intersection: %s\n", rcv_msg.intersection);
            strncpy(snd_msg.action, "ERROR", sizeof(snd_msg.action));
        }
        else if (strncmp(rcv_msg.action, "ACQUIRE", 7) == 0) {
            // Try to add the train as a holder; if not, enqueue the train
            if (add_holder(shared_intersections, idx, rcv_msg.train_id)) {
                // Successfully added, send GRANT
                strncpy(snd_msg.action, "GRANT", sizeof(snd_msg.action));
            } else {
                // At capacity, so enqueue the train and send WAIT
                enqueue_waiter(shared_intersections, idx, rcv_msg.train_id);
                strncpy(snd_msg.action, "WAIT", sizeof(snd_msg.action));
            }
        } else if (strncmp(rcv_msg.action, "RELEASE", 7) == 0) {
            // Remove the train from the holders list
            if (remove_holder(shared_intersections, idx, rcv_msg.train_id)) {
                // If there is a waiting train, dequeue it and grant access.
                int next_train = dequeue_waiter(shared_intersections, idx);
                if (next_train != -1) {
                    // Add the waiting train as a holder.
                    add_holder(shared_intersections, idx, next_train);
                    // Prepare and send a GRANT message to the waiting train
                    Message grant_msg;
                    memset(&grant_msg, 0, sizeof(grant_msg));
                    grant_msg.mtype = next_train + 100;
                    grant_msg.train_id = next_train;
                    strncpy(grant_msg.action, "GRANT", sizeof(grant_msg.action));
                    strncpy(grant_msg.intersection, rcv_msg.intersection, MAX_NAME);
                    if (msgsnd(msgid, &grant_msg, sizeof(Message) - sizeof(long), 0) == -1) {
                        perror("[SERVER] msgsnd for granting waiting train");
                    } else {
                        printf("[SERVER] Granted waiting train %d access to %s\n", next_train, rcv_msg.intersection);
                    }
                }
                strncpy(snd_msg.action, "OK", sizeof(snd_msg.action));
            } else {
                strncpy(snd_msg.action, "ERROR", sizeof(snd_msg.action));
            }
        } else {
            strncpy(snd_msg.action, "UNKNOWN", sizeof(snd_msg.action));
        }
     }
     
     // remove the message queue before exiting
     if (msgctl(msgid, IPC_RMID, NULL) == -1) {
         perror("[SERVER] msgctl(IPC_RMID)");
         exit(EXIT_FAILURE);
     }
     printf("[SERVER] Message queue removed. Exiting.\n");
>>>>>>> Stashed changes

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