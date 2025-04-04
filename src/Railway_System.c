#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#include "Basic_IPC_Workflow/ipc.h" // Zach Oyer
#include "parser/parser.h" // Jarett Woodard
#include "Basic_IPC_Workflow/intersection_locks.h" // Jake Pinell
#include "Shared_Memory_Setup/Memory_Segments.h" // Steve Kuria

//This file uses code from server.c authored by Jason Greer

int main(){
    // Initialize shared memory for intersections
    size_t shm_size;
    SharedIntersection *shared_intersections = init_shared_memory("/intersection_shm", &shm_size);
    if (!shared_intersections) {
        fprintf(stderr, "[SERVER] Failed to initialize shared memory.\n");
        return 1;
    }
    printf("[SERVER] Shared memory initialized.\n");

    // create array of train structs and count
    TrainEntry trains[LINE_MAX];
    int trainCount = getTrains(trains);

    // create array of intersections tructs and count
    IntersectionEntry intersections[LINE_MAX];
    int intersectionCount = getIntersections(intersections);
    
    // PRINT CHECK. 
    // Print individual train entries
    printf("Train Entries:\n"); 
    printTrainEntries(trains, trainCount);

    // Print all train entries
    printf("Intersection Entries:\n");
    printIntersectionEntries(intersections, intersectionCount);

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
         
         if (strncmp(rcv_msg.action, "ACQUIRE", 7) == 0) {
             strncpy(snd_msg.action, "GRANT", sizeof(snd_msg.action));
         } else if (strncmp(rcv_msg.action, "RELEASE", 7) == 0) {
             strncpy(snd_msg.action, "OK", sizeof(snd_msg.action));
         } else {
             strncpy(snd_msg.action, "UNKNOWN", sizeof(snd_msg.action));
         }
         
         // send the response message
         if (msgsnd(msgid, &snd_msg, sizeof(Message) - sizeof(long), 0) == -1) {
             perror("[SERVER] msgsnd");
         } else {
             printf("[SERVER] Sent response: Train %d \"%s\" on %s\n",
                    snd_msg.train_id, snd_msg.action, snd_msg.intersection);
         }
     }
     
     // remove the message queue before exiting
     if (msgctl(msgid, IPC_RMID, NULL) == -1) {
         perror("[SERVER] msgctl(IPC_RMID)");
         exit(EXIT_FAILURE);
     }
     printf("[SERVER] Message queue removed. Exiting.\n");

     // Cleanup Shared memory
    destroy_shared_memory(shared_intersections, "/intersection_shm", shm_size);
    printf("[SERVER] Shared memory cleaned up.\n");
 
     return 0;
}
 
