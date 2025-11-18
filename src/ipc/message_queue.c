#include "message_queue.h"


int create_queue(key_t k) {
    int qid = msgget(k, IPC_CREAT | 0600);
    if(qid == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }

    return qid;
}

void send_msg(int qid, struct message_t *msg) {
    if(msgsnd(qid, msg, MSG_LENGTH, 0) < 0) {
        ERROR
        exit(EXIT_FAILURE);
    }

}

void receive_msg(int qid, struct message_t *msg, long mtype) {
    if(msgrcv(qid, msg, MSG_LENGTH, mtype, 0) < 0) {
        ERROR
        exit(EXIT_FAILURE);
    }
}

void delete_queue(int qid) {
    if(msgctl(qid, IPC_RMID, NULL) == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }
}