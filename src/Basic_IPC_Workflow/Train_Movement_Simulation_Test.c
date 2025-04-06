// Author Jake Richardson
// Description: This is a version of Train_Movement_Simulation by Zachary Oyer with added print statements
// for the means of testing communication queues as well as the train forking process.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include "intersection_locks.h"

#define MSG_KEY 1234
#define MAX_NAME 64
#define MAX_INTERSECTIONS 10
// Array to store intersection data
Intersection intersections[MAX_INTERSECTIONS];
int num_intersections = 0;

typedef struct
{
    long mtype;
    int train_id;
    char intersection[MAX_NAME];
    char action[8]; // "ACQUIRE" or "RELEASE"
} Message;

void send_msg(int msgid, int train_id, const char *intersection, const char *action)
{
    Message msg;
    msg.mtype = 1;
    msg.train_id = train_id;
    strncpy(msg.intersection, intersection, MAX_NAME);
    strncpy(msg.action, action, sizeof(msg.action));
    msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
}

void send_msg(int msgid, int train_id, const char *intersection, const char *action)
{
    Message msg;
    msg.mtype = 1;
    msg.train_id = train_id;
    strncpy(msg.intersection, intersection, MAX_NAME);
    strncpy(msg.action, action, sizeof(msg.action));

    if (msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0) == -1)
    {
        perror("[TRAIN] Failed to send message");
    }
    else
    {
        printf("[TRAIN %d] Sent message: %s %s\n", train_id, action, intersection);
    }
}

// Initialize intersections with their capacities
void init_intersections()
{
    // IntersectionA - capacity 1 (mutex)
    strcpy(intersections[0].name, "IntersectionA");
    intersections[0].capacity = 1;
    init_mutex_lock(&intersections[0]);

    // IntersectionB - capacity 2 (semaphore)
    strcpy(intersections[1].name, "IntersectionB");
    intersections[1].capacity = 2;
    init_semaphore_lock(&intersections[1]);

    // IntersectionC - capacity 1 (mutex)
    strcpy(intersections[2].name, "IntersectionC");
    intersections[2].capacity = 1;
    init_mutex_lock(&intersections[2]);

    // IntersectionD - capacity 3 (semaphore)
    strcpy(intersections[3].name, "IntersectionD");
    intersections[3].capacity = 3;
    init_semaphore_lock(&intersections[3]);

    // IntersectionE - capacity 1 (mutex)
    strcpy(intersections[4].name, "IntersectionE");
    intersections[4].capacity = 1;
    init_mutex_lock(&intersections[4]);

    num_intersections = 5;
    printf("Initialized %d intersections\n", num_intersections);
}

// Find an intersection by name
int find_intersection(const char *name)
{
    for (int i = 0; i < num_intersections; i++)
    {
        if (strcmp(intersections[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1; // Not found
}

void server_loop(int msgid)
{
    Message msg;

    while (1)
    {
        if (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), 0, 0) > 0)
        {
            printf("[SERVER] %s from Train %d for %s\n", msg.action, msg.train_id, msg.intersection);

            int idx = find_intersection(msg.intersection);
            if (idx == -1)
            {
                printf("[SERVER] Intersection %s not found\n", msg.intersection);
                continue;
            }

            if (strcmp(msg.action, "ACQUIRE") == 0)
            {
                // Try to acquire the lock
                if (acquire_lock(&intersections[idx]) == 0)
                {
                    printf("[SERVER] Granted %s to Train %d\n", msg.intersection, msg.train_id);
                }
                else
                {
                    printf("[SERVER] Could not grant %s to Train %d\n", msg.intersection, msg.train_id);
                }
            }
            else if (strcmp(msg.action, "RELEASE") == 0)
            {
                // Release the lock
                if (release_lock(&intersections[idx]) == 0)
                {
                    printf("[SERVER] Released %s from Train %d\n", msg.intersection, msg.train_id);
                }
                else
                {
                    printf("[SERVER] Failed to release %s from Train %d\n", msg.intersection, msg.train_id);
                }
            }
        }
    }
}

int main()
{
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);

    // Initialize the intersections with their locks
    init_intersections();

    const char *route1[] = {"IntersectionA", "IntersectionB", "IntersectionC"};
    const char *route2[] = {"IntersectionB", "IntersectionD", "IntersectionE"};

    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        printf("[TRAIN 1] Process started (PID: %d)\n", getpid());
        run_train(msgid, 1, route1, 3);
        exit(0);
    }
    else if (pid1 > 0)
    {
        printf("[MAIN] Forked Train 1 (PID: %d)\n", pid1);
    }
    else
    {
        perror("[MAIN] Failed to fork Train 1");
    }

    pid_t pid2 = fork();
    if (pid2 == 0)
    {
        printf("[TRAIN 2] Process started (PID: %d)\n", getpid());
        run_train(msgid, 2, route2, 3);
        exit(0);
    }
    else if (pid2 > 0)
    {
        printf("[MAIN] Forked Train 2 (PID: %d)\n", pid2);
    }
    else
    {
        perror("[MAIN] Failed to fork Train 2");
    }

    server_loop(msgid); // run forever for now

    // This cleanup code won't be reached in this example,
    // but good practice to include it
    for (int i = 0; i < num_intersections; i++)
    {
        cleanup_locks(&intersections[i]);
    }

    return 0;
}
