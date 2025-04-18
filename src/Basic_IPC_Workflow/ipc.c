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
#include "ipc.h" // Header that defines the Message struct and send_message prototype

// Sends a message from a train to the central server
// Includes the train ID, the intersection being requested or released,
// and the action ("ACQUIRE" or "RELEASE")
void send_message(int msgid, int train_id, const char* intersection, const char* action) {
    Message msg;

    msg.mtype = 1;  // All messages have type 1; can be expanded later for prioritization
    msg.train_id = train_id; // Set the sender train's ID

    // Copy the intersection name into the message (safe bounded copy)
    strncpy(msg.intersection, intersection, MAX_NAME);

    // Copy the action ("ACQUIRE" or "RELEASE") into the message
    strncpy(msg.action, action, sizeof(msg.action));

    // Send the message to the System V message queue
    // sizeof(Message) - sizeof(long) because mtype is not included in message size
    if (msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
        // Print an error if the message could not be sent
        perror("msgsnd failed");
    }
}


