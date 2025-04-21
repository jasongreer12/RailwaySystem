/*
Author: Jarett Woodard
Group: B
Email: jarett.woodard@okstate.edu
Date: 4.4.2025
Updated 4/13/2025
*/

/*Timestamping known working condition before merge to main. 4.20.2025 8:56PM CDT*/
/*Timestamping known working condition with all branches merged before merge to main 4.20.2025 9:08 CDT*/
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "logger/logger.h"                                // Jason Greer
#include "logger/csv_logger.h"                            // Jarett Woodard
#include "Basic_IPC_Workflow/ipc.h"                       // Zachary Oyer
#include "parser/parser.h"                                // Jarett Woodard
#include "Basic_IPC_Workflow/intersection_locks.h"        // Jake Pinell
#include "Shared_Memory_Setup/Memory_Segments.h"          // Steve Kuria
#include "Basic_IPC_Workflow/resource_allocation_graph.h" // Zachary Oyer
#include "Basic_IPC_Workflow/fake_sec.h"                  // Jake Pinell

// This file uses code from server.c authored by Jason Greer

#define LINE_MAX 256
// #define TIME_SHM_NAME "/sim_time_shm"

// helper to map intersection name in index in iEntries[]
static int find_intersection_index(const IntersectionEntry entries[], int count, const char *name)
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

// returns true if train is currently a holder of intersection in shared memory
static bool isHolder(SharedIntersection *shared_intersections, int idx, int train_id)
{
    if (remove_holder(shared_intersections, idx, train_id))
    {
        // put it back as we only removed it to test if it exists
        add_holder(shared_intersections, idx, train_id);
        return true;
    }
    return false;
}

int main()
{
    // initialize both loggers
    log_init("simulation.log", 1);
    // LOG_SERVER("Initializing Train Movement Simulation");

    FILE *csv_file = csv_logger_init();
    if (!csv_file)
    {
        LOG_SERVER("Failed to initialize CSV logger");
        fprintf(stderr, "[SERVER] Failed to initialize CSV logger.\n");
        exit(1);
    }

    int nextAcquire[LINE_MAX] = {0}; // var used in tracking trains route for deadlock resolution

    // log system startup using system message
    Message sys_msg = {
        .mtype = 1,
        .train_id = 0};
    strncpy(sys_msg.intersection, "SYSTEM", MAX_NAME);
    strncpy(sys_msg.action, "STARTUP", sizeof(sys_msg.action) - 1);
    sys_msg.action[sizeof(sys_msg.action) - 1] = '\0';

    // Initialize simulated clock
    // size_t time_size;
    // TimeKeeper *clk = init_time(TIME_SHM_NAME, &time_size);
    // if (!clk) { fprintf(stderr, "Failed time shm\n"); exit(1); }

    // set up shared memory
    // MOVED TO LOGGER FILE
    // size_t shm_size;
    // SharedIntersection *shared_intersections =
    //     init_shared_memory("/intersection_shm", &shm_size);
    // if (!shared_intersections)
    // {
    //     LOG_SERVER("Failed to initialize shared memory");
    //     fprintf(stderr, "[SERVER] Failed to initialize shared memory.\n");
    //     exit(1);
    // }
    // LOG_SERVER("Shared memory initialized");

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
    // LOG_SERVER("Shared memory initialized");

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
            log_event("Initialized mutex for %s (capacity=1)", locks[i].name);
            // LOG_SERVER("Initialized mutex for %s (capacity=1)", locks[i].name);
        }
        else
        {
            init_semaphore_lock(&locks[i]);
            log_event("Initialized semaphore for %s (capacity=%d)", locks[i].name, locks[i].capacity);
            // LOG_SERVER("Initialized semaphore for %s (capacity=%d)",
            // locks[i].name, locks[i].capacity);
        }
    }
    LOG_SERVER("\n");

    // set up message queue
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0)
    {
        LOG_SERVER(" msgget failed: %s", strerror(errno));
        perror("[SERVER] msgget");
        exit(1);
    }
    // LOG_SERVER("Message queue ready (ID: %d)", msgid);
    printf("%s [SERVER] Message queue ready (ID: %d)\n", getFakeTime(), msgid);

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

        // prepare common parts of response
        memset(&resp, 0, sizeof(resp));
        resp.mtype = req.train_id + 100;
        resp.train_id = req.train_id;
        strncpy(resp.intersection, req.intersection, sizeof(resp.intersection) - 1);

        // increments time in shared memory through logger.h
        setFakeSec(1);
        // Logs request. gettime is called inside the macro
        LOG_SERVER("Received: Train %d requests \"%s\" on %s", req.train_id, req.action, req.intersection);

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
                // attempt to add the train as a holder in shared memory. If successful, try to acquire the local lock. Otherwise, put train in exit queue.
                if (add_holder(shared_intersections, idx, req.train_id))
                    // need to use resource allocation graph here with this function: add_request_edge(int train_id, const char* intersection)
                    add_request_edge(req.train_id, req.intersection);
                print_graph();
                // attempt to add the train as a holder in shared memory. If successful, try to acquire the local lock. Otherwise, enqueue the train to wait.
                // 8:08 i think were having an issue with trains requesting an intersection that is already occupied
                if (isHolder(shared_intersections, idx, req.train_id))
                {
                    strncpy(resp.action, "GRANT", sizeof(resp.action) - 1);
                    LOG_SERVER("ReGRANT %s to Train %d (already held)", req.intersection, req.train_id);
                }
                else if (add_holder(shared_intersections, idx, req.train_id))
                {

                    int result = acquire_lock(&locks[idx]);
                    if (req.train_id == 1)
                    {
                        printf("\n%d\n", result);
                    }
                    if (result == 0)
                    {
                        strncpy(resp.action, "GRANT", sizeof(resp.action) - 1);
                        nextAcquire[resp.train_id - 1]++;
                        // now = increment_time(clk, 1);
                        // format_timestamp(now, ts);
                        setFakeSec(1);
                        add_allocation_edge(req.train_id, req.intersection); // function automatically removes the request and adds the allocation
                        LOG_SERVER("GRANTED %s to Train %d", req.intersection, req.train_id);
                    }
                    else
                    {
                        // if local lock acquisition fails remove the holder and queue the train
                        remove_holder(shared_intersections, idx, req.train_id);
                        enqueue_waiter(shared_intersections, idx, req.train_id);
                        strncpy(resp.action, "WAIT", sizeof(resp.action) - 1);
                        LOG_SERVER("WAITING: Local lock error, Train %d queued for %s",
                                   req.train_id, req.intersection);
                        // now = increment_time(clk, 1);
                        // format_timestamp(now, ts);
                        setFakeSec(1);
                        LOG_SERVER("%s WAITING: Local lock error, Train %d queued for %s", req.train_id, req.intersection);
                    }
                }
                else
                {
                    printf("WERE IN THE INTERSECTION FULL BRANCH with train%d\n", req.train_id);
                    // intersection at capacity add the train to the waiting queue
                    enqueue_waiter(shared_intersections, idx, req.train_id);
                    strncpy(resp.action, "WAIT", sizeof(resp.action) - 1);
                    LOG_SERVER("WAITING: full, Train %d queued for %s", req.train_id, req.intersection);
                    // now = increment_time(clk, 1);
                    // format_timestamp(now, ts);
                    setFakeSec(1);
                    LOG_SERVER("%s is full. Train %d added to wait queue.", req.intersection, req.train_id);
                    if (detect_deadlock())
                    { // check for deadlock since intersection is at capacity
                        LOG_SERVER("Deadlock detected! Cycle: Train%d <> Train%d.\n", req.train_id, req.train_id);
                        // need to add logic here to remove intersection from train
                        // get intersection index involved with deadlock
                        // get the train that has desired intersection intersection
                        int victim = -1;
                        for (int i = 1; i <= trainCount; i++)
                        {
                            if (remove_holder(shared_intersections, idx, i))
                            {
                                victim = i;
                                break;
                            }
                        }
                        if (victim > 0)
                        {
                            // preempt that one lock
                            release_lock(&locks[idx]);
                            remove_edges(victim, req.intersection);
                            setFakeSec(1);
                            LOG_SERVER("Preemptively released %s from Train %d",
                                       req.intersection, victim);

                            // grant to the requester
                            add_holder(shared_intersections, idx, req.train_id);
                            nextAcquire[req.train_id - 1]++;
                            add_allocation_edge(req.train_id, req.intersection);
                            strncpy(resp.action, "GRANT", sizeof(resp.action) - 1);
                            setFakeSec(1);
                            LOG_SERVER("Forced GRANT of %s to Train %d",
                                       req.intersection, req.train_id);
                        }
                    }
                }
            }

            else // RELEASE
            {
                int result = release_lock(&locks[idx]);
                if (result == 0)
                {
                    if (remove_holder(shared_intersections, idx, req.train_id))
                    {

                        strncpy(resp.action, "OK", sizeof(resp.action) - 1);
                        LOG_SERVER("Released %s from Train %d", req.intersection, req.train_id);

                        // check of any trains are waiting
                        int next_train = dequeue_waiter(shared_intersections, idx);
                        if (next_train != -1)
                        {
                            if (add_holder(shared_intersections, idx, next_train))
                            {
                                result = acquire_lock(&locks[idx]);
                                if (result == 0)
                                {
                                    // Send GRANT to waiting train
                                    Message grant_msg;
                                    memset(&grant_msg, 0, sizeof(grant_msg));
                                    grant_msg.mtype = next_train + 100;
                                    grant_msg.train_id = next_train;
                                    strncpy(grant_msg.intersection,
                                            req.intersection,
                                            sizeof(grant_msg.intersection) - 1);
                                    strncpy(grant_msg.action, "GRANT",
                                            sizeof(grant_msg.action) - 1);

                                    if (msgsnd(msgid, &grant_msg,
                                               sizeof(grant_msg) - sizeof(long), 0) == -1)
                                    {
                                        LOG_SERVER("msgsnd(GRANT) to Train %d failed: %s",
                                                   next_train, strerror(errno));
                                    }
                                    else
                                    {
                                        setFakeSec(1); // Increment time when granting to waiting train
                                        LOG_SERVER("Granted %s to waiting Train %d",
                                                   req.intersection, next_train);
                                    }
                                }
                                else
                                {
                                    // If lock acquisition fails, put train back in queue
                                    remove_holder(shared_intersections, idx, next_train);
                                    enqueue_waiter(shared_intersections, idx, next_train);
                                }
                            }
                        }
                        // grant the waiting train by adding it as a holder
                        // add_holder(shared_intersections, idx, next_train);
                        // LOG_SERVER("Granted waiting train %d for %s", next_train, req.intersection);
                        // strncpy(resp.action, "GRANT", sizeof(resp.action) - 1); // send grant message to train waiting
                        add_holder(shared_intersections, idx, next_train);
                        // now = increment_time(clk, 1);
                        // format_timestamp(now, ts);
                        setFakeSec(1);
                        LOG_SERVER("Granted waiting train %d", next_train, req.intersection);

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
                            setFakeSec(1);
                            LOG_SERVER("msgsnd(GRANT) to Train %d failed: %s",
                                       next_train, strerror(errno));
                            char errBuff[64];
                            snprintf(errBuff, sizeof(errBuff), "%s [SERVER] msgsnd(GRANT) to Train %d failed: %s", getFakeTime(), next_train, strerror(errno));
                            perror(errBuff);
                        }
                        else
                        {
                            setFakeSec(1);
                            LOG_SERVER("Sent response: Train %d \"GRANT\" on %s",
                                       next_train, req.intersection);
                            printf("%s [SERVER] Sent response: Train %d \"GRANT\" on %s\n",
                                   getFakeTime(), next_train, req.intersection);
                        }
                    }

                    strncpy(resp.action, "OK", sizeof(resp.action) - 1);
                    // now = increment_time(clk, 1);
                    // format_timestamp(now, ts);
                    setFakeSec(1);
                    LOG_SERVER("Released %s from Train %d", req.intersection, req.train_id);
                }
                else
                {
                    strncpy(resp.action, "FAIL", sizeof(resp.action) - 1);
                    LOG_SERVER("Failed to remove Train %d from holders of %s",
                               req.train_id, req.intersection);
                    LOG_SERVER("Failed to remove Train %d from holders of %s", req.train_id, req.intersection);
                }
            }
            // else
            // {
            //     strncpy(resp.action, "FAIL", sizeof(resp.action) - 1);
            //     LOG_SERVER("Failed to release %s from Train %d",
            //              req.intersection, req.train_id);
            // }
        }
    }

    // Only increment time when sending final response if we're changing state
    if (strcmp(resp.action, "GRANT") == 0 || strcmp(resp.action, "OK") == 0)
    {
        setFakeSec(1);
    }

    if (msgsnd(msgid, &resp, sizeof(resp) - sizeof(long), 0) == -1)
    {
        LOG_SERVER("msgsnd failed: %s", strerror(errno));
        perror("[SERVER] msgsnd");
    }
    else
    {
        LOG_SERVER("Sent response: Train %d \"%s\" on %s",
                   resp.train_id, resp.action, resp.intersection);
        printf("%s [SERVER] Sent response: Train %d \"%s\" on %s\n", getFakeTime(),
               resp.train_id, resp.action, resp.intersection);
    }

    // clean the queue only after receiving STOP signal
    if (msgctl(msgid, IPC_RMID, NULL) == -1)
    {
        LOG_SERVER("msgctl(IPC_RMID) failed: %s", strerror(errno));
        perror("[SERVER] msgctl");
    }
    else
    {
        LOG_SERVER("Message queue removed");
        printf("%s [SERVER] Message queue removed. Exiting.\n", getFakeTime());
    }

    // Give train simulator time to clean up its resources
    sleep(1);

    // final cleanup
    LOG_SERVER("SIMULATION COMPLETE. All trains reached destinations.");
    log_close();
    exit(0); // terminate
}