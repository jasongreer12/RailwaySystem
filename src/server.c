// Jason Greer
// jason.greer@okstate.edu
// 4/3/25
// GROUP: B
// This file handles ACQUIRE/RELEASE requests from trains via message queues and grants access to intersections.

// server.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include "Basic_IPC_Workflow/ipc.h"  // this header contains the Message struct & MSG_KEY

int main(void) {
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

    return 0;
}

