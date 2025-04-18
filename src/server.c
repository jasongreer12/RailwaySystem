// Jason Greer
// jason.greer@okstate.edu
// 4/18/25
// GROUP: B
// This file handles ACQUIRE/RELEASE requests from trains via message queues and grants access to intersections.

// server.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include "Basic_IPC_Workflow/ipc.h" // this header contains the Message struct & MSG_KEY
/* commented out because this file isnt actually used. Just shows progress
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
    // initialize both loggers
    log_init("simulation.log", 1);
    LOG_SERVER("Initializing Train Movement Simulation");

    // log system startup using system message
    Message sys_msg = {
        .mtype = 1,
        .train_id = 0};
    strncpy(sys_msg.intersection, "SYSTEM", MAX_NAME);
    strncpy(sys_msg.action, "STARTUP", sizeof(sys_msg.action) - 1);
    sys_msg.action[sizeof(sys_msg.action) - 1] = '\0';

    // set up shared memory
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
        // copy name & capacity
        strncpy(locks[i].name, iEntries[i].id, MAX_NAME_LENGTH - 1);
        locks[i].name[MAX_NAME_LENGTH - 1] = '\0';
        locks[i].capacity = iEntries[i].capacity;

        // initialize the lock
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

        // if STOP then break
        if (strcmp(req.action, "STOP") == 0)
        {
            LOG_SERVER("Received STOP signal. Exiting server loop");
            break;
        }

        LOG_SERVER("Received: Train %d requests \"%s\" on %s",
                   req.train_id, req.action, req.intersection);

        // prepare common parts of response
        memset(&resp, 0, sizeof(resp));
        resp.mtype = req.train_id + 100;
        resp.train_id = req.train_id;
        strncpy(resp.intersection, req.intersection, sizeof(resp.intersection) - 1);

        // find which lock to use
        int idx = find_intersection_index(iEntries, intersectionCount, req.intersection);
        if (idx < 0)
        {
            strncpy(resp.action, "FAIL", sizeof(resp.action) - 1);
            LOG_SERVER("Unknown intersection %s", req.intersection);
        }
        else
        {
            // process ACQUIRE or RELEASE on locks[idx] and update shared memory tracking
            if (strcmp(req.action, "ACQUIRE") == 0)
            {
                // attempt to add the train as a holder in shared memory. If successful, try to acquire the local lock. Otherwise, enqueue the train to wait.
                if (add_holder(shared_intersections, idx, req.train_id))
                {
                    int result = acquire_lock(&locks[idx]);

                    if (result == 0)
                    {
                        strncpy(resp.action, "GRANT", sizeof(resp.action) - 1);
                        LOG_SERVER("GRANTED %s to Train %d", req.intersection, req.train_id);
                    }
                    else
                    {
                        // if local lock acquisition fails remove the holder and queue the train
                        remove_holder(shared_intersections, idx, req.train_id);
                        enqueue_waiter(shared_intersections, idx, req.train_id);
                        strncpy(resp.action, "WAIT", sizeof(resp.action) - 1);
                        LOG_SERVER("WAITING: Local lock error, Train %d queued for %s", req.train_id, req.intersection);
                    }
                }
                else
                {
                    // intersection at capacity add the train to the waiting queue
                    enqueue_waiter(shared_intersections, idx, req.train_id);
                    strncpy(resp.action, "WAIT", sizeof(resp.action) - 1);
                    LOG_SERVER("WAITING: %s full, Train %d queued", req.intersection, req.train_id);
                }
            }
            else // RELEASE
            {
                Message grant_msg; // grant message to be sent to waiting train later
                int result = release_lock(&locks[idx]);
                int next_train;

                if (result == 0)
                {
                    if (remove_holder(shared_intersections, idx, req.train_id))
                    {
                        // after releasing, check if any train is waiting for this intersection
                        int next_train = dequeue_waiter(shared_intersections, idx);
                        if (next_train != -1)
                        {
                            // grant the waiting train by adding it as a holder
                            // add_holder(shared_intersections, idx, next_train);
                            // LOG_SERVER("Granted waiting train %d for %s", next_train, req.intersection);
                            // strncpy(resp.action, "GRANT", sizeof(resp.action) - 1); // send grant message to train waiting
                            add_holder(shared_intersections, idx, next_train);
                            LOG_SERVER("Granted waiting train %d for %s", next_train, req.intersection);

                            // send a  GRANT message to that train whose waiting
                            Message grant_msg;
                            memset(&grant_msg, 0, sizeof(grant_msg));
                            grant_msg.mtype = next_train + 100; // client is listening on train_id+100
                            grant_msg.train_id = next_train;
                            strncpy(grant_msg.intersection,
                                    req.intersection,
                                    sizeof(grant_msg.intersection) - 1);
                            snprintf(grant_msg.action,
                                     sizeof(grant_msg.action),
                                     "GRANT");

                            if (msgsnd(msgid,
                                       &grant_msg,
                                       sizeof(grant_msg) - sizeof(long),
                                       0) == -1)
                            {
                                LOG_SERVER("msgsnd(GRANT) to Train %d failed: %s",
                                           next_train, strerror(errno));
                                perror("[SERVER] msgsnd GRANT");
                            }
                            else
                            {
                                LOG_SERVER("Sent response: Train %d \"GRANT\" on %s",
                                           next_train, req.intersection);
                                printf("[SERVER] Sent response: Train %d \"GRANT\" on %s\n",
                                       next_train, req.intersection);
                            }
                        }

                        strncpy(resp.action, "OK", sizeof(resp.action) - 1);
                        LOG_SERVER("Released %s from Train %d", req.intersection, req.train_id);
                    }
                    else
                    {
                        strncpy(resp.action, "FAIL", sizeof(resp.action) - 1);
                        LOG_SERVER("Failed to remove Train %d from holders of %s", req.train_id, req.intersection);
                    }
                }
                else
                {
                    strncpy(resp.action, "FAIL", sizeof(resp.action) - 1);
                    LOG_SERVER("Failed to release %s from Train %d", req.intersection, req.train_id);
                }
            }
        }

        // send response
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

    // final system state

    // close all loggers
    LOG_SERVER("SIMULATION COMPLETE. All trains reached destinations.");
    log_close();
    exit(0); // terminate
}*/