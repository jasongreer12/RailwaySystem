// ipc.c
// Author: Zachary Oyer
// Date: 4-4-2025
// Group B
// Implements the send_message() function for sending ACQUIRE and RELEASE
// messages using System V message queues. Used by train processes to request
// and release intersections.
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include "ipc.h"

void send_message(int msgid, int train_id, const char* intersection, const char* action) {
    Message msg;
    msg.mtype = 1;  // all messages go into one queue for now
    msg.train_id = train_id;
    strncpy(msg.intersection, intersection, MAX_NAME);
    strncpy(msg.action, action, sizeof(msg.action));

    if (msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
        perror("msgsnd failed");
    }
}


