// ipc.h
// Author: Zachary Oyer
// Group: B
// Email: zachary.oyer@okstate.edu
// Date: 4-4-2025
// for System V IPC used to simulate train-intersection communication.
#ifndef IPC_H
#define IPC_H

#define MAX_NAME 64
#define MSG_KEY 1234

typedef struct {
    long mtype;// required for System V message queues
    int train_id;
    char intersection[MAX_NAME];
    char action[8];// "ACQUIRE" or "RELEASE"
} Message;

// Send an ACQUIRE or RELEASE message to the server
void send_message(int msgid, int train_id, const char* intersection, const char* action);

#endif
