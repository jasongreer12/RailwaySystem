/* Note: we needed to add author for this file and Jarett created it
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

int main()
{
    log_init("simulation.log");

    // Initialize shared memory for intersections
    size_t shm_size;
    SharedIntersection *shared_intersections = init_shared_memory("/intersection_shm", &shm_size);
    if (!shared_intersections)
    {
        fprintf(stderr, "[SERVER] Failed to initialize shared memory.\n");
        return 1;
    }
    printf("[SERVER] Shared memory initialized.\n");
    LOG_SERVER("Shared memory initialized.");

    // Create array of train structs and count
    TrainEntry trains[LINE_MAX];
    int trainCount = getTrains(trains);

    // Create array of intersections structs and count
    IntersectionEntry intersections[LINE_MAX];
    int intersectionCount = getIntersections(intersections);

    // PRINT CHECK.
    // Print individual train entries
    LOG_SERVER("Parsed %d trains from file", trainCount);
    printf("Train Entries:\n");
    printTrainEntries(trains, trainCount);

    // Print all intersection entries
    printf("Intersection Entries:\n");
    LOG_SERVER("Parsed %d intersections from file", intersectionCount);
    printIntersectionEntries(intersections, intersectionCount);

    // Create (or open) the message queue using the defined MSG_KEY
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0)
    {
        LOG_SERVER("Error occurred in railway_system.c line 58.");
        perror("[SERVER] msgget");
        exit(EXIT_FAILURE);
    }
    printf("[SERVER] Message queue ready (ID: %d)\n", msgid);
    LOG_SERVER("Message queue ready (ID: %d)", msgid);

    Message rcv_msg, snd_msg;
    while (1)
    {
        // Wait for any message of type 1 (ACQUIRE/RELEASE requests)
        if (msgrcv(msgid, &rcv_msg, sizeof(Message) - sizeof(long), 1, 0) == -1)
        {
            perror("[SERVER] msgrcv");
            continue;
        }

        printf("[SERVER] Received: Train %d requests \"%s\" on %s\n",
               rcv_msg.train_id, rcv_msg.action, rcv_msg.intersection);
        LOG_SERVER("Received: Train %d requests \"%s\" on %s",
                   rcv_msg.train_id,
                   rcv_msg.action,
                   rcv_msg.intersection);

        // Check for a stop condition
        if (strncmp(rcv_msg.action, "STOP", 4) == 0)
        {
            printf("[SERVER] Received STOP signal. Shutting down.\n");
            LOG_SERVER("Received STOP signal. Shutting down.");
            break;
        }

        // Prepare the response message:
        // Set the response mtype to the train's id so the train can receive it
        memset(&snd_msg, 0, sizeof(snd_msg));
        snd_msg.mtype = rcv_msg.train_id + 100; // Response addressed to the requesting train
        snd_msg.train_id = rcv_msg.train_id;
        strncpy(snd_msg.intersection, rcv_msg.intersection, MAX_NAME);

        if (strncmp(rcv_msg.action, "ACQUIRE", 7) == 0)
        {
            strncpy(snd_msg.action, "GRANT", sizeof(snd_msg.action));
        }
        else if (strncmp(rcv_msg.action, "RELEASE", 7) == 0)
        {
            strncpy(snd_msg.action, "OK", sizeof(snd_msg.action));
        }
        else
        {
            strncpy(snd_msg.action, "UNKNOWN", sizeof(snd_msg.action));
        }

        // Send the response message
        if (msgsnd(msgid, &snd_msg, sizeof(Message) - sizeof(long), 0) == -1)
        {
            perror("[SERVER] msgsnd");
            LOG_SERVER("Failed to send response to Train %d: %s",
                       snd_msg.train_id,
                       strerror(errno));
        }
        else
        {
            printf("[SERVER] Sent response: Train %d \"%s\" on %s\n",
                   snd_msg.train_id, snd_msg.action, snd_msg.intersection);
            LOG_SERVER("Sent response: Train %d \"%s\" on %s",
                       snd_msg.train_id,
                       snd_msg.action,
                       snd_msg.intersection);
        }
    }

    // Remove the message queue before exiting
    if (msgctl(msgid, IPC_RMID, NULL) == -1)
    {
        perror("[SERVER] msgctl(IPC_RMID)");
        exit(EXIT_FAILURE);
    }
    printf("[SERVER] Message queue removed. Exiting.\n");
    LOG_SERVER("Message queue removed. Exiting.");

    // Cleanup shared memory
    destroy_shared_memory(shared_intersections, "/intersection_shm", shm_size);
    printf("[SERVER] Shared memory cleaned up.\n");

    LOG_SERVER("SIMULATION COMPLETE. All trains reached destinations.");
    log_close();
    return 0;
}
