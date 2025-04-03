// Author Zachary Oyer
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>

#define MSG_KEY 1234
#define MAX_NAME 64

typedef struct {
    long mtype;
    int train_id;
    char intersection[MAX_NAME];
    char action[8]; // "ACQUIRE" or "RELEASE"
} Message;

void send_msg(int msgid, int train_id, const char* intersection, const char* action) {
    Message msg;
    msg.mtype = 1;
    msg.train_id = train_id;
    strncpy(msg.intersection, intersection, MAX_NAME);
    strncpy(msg.action, action, sizeof(msg.action));
    msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0);
}

void run_train(int msgid, int train_id, const char* route[], int route_len) {
    for (int i = 0; i < route_len; i++) {
        send_msg(msgid, train_id, route[i], "ACQUIRE");
        printf("Train %d requesting %s\n", train_id, route[i]);
        sleep(1); // simulate waiting

        printf("Train %d moving through %s\n", train_id, route[i]);
        sleep(1); // simulate movement

        send_msg(msgid, train_id, route[i], "RELEASE");
        printf("Train %d released %s\n", train_id, route[i]);
    }
}

void server_loop(int msgid) {
    Message msg;
    while (1) {
        if (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), 0, 0) > 0) {
            printf("[SERVER] %s from Train %d for %s\n", msg.action, msg.train_id, msg.intersection);
        }
    }
}

int main() {
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);

    const char* route1[] = {"IntersectionA", "IntersectionB", "IntersectionC"};
    const char* route2[] = {"IntersectionB", "IntersectionD", "IntersectionE"};

    pid_t pid1 = fork();
    if (pid1 == 0) {
        run_train(msgid, 1, route1, 3);
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        run_train(msgid, 2, route2, 3);
        exit(0);
    }

    server_loop(msgid);  // run forever for now

    return 0;
}
