#include "message_queue.h"


int create_queue(key_t k) {
    int qid = msgget(k, IPC_CREAT | IPC_EXCL | 0600);
    if(qid == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }

    return qid;
}

void send_msg(int qid, Messaggio* msg) {
    if(msgsnd(qid, msg, sizeof(Messaggio)-sizeof(long), 0) < 0) {
        ERROR
        exit(EXIT_FAILURE);
    }

}

void receive_msg(int qid, Messaggio* msg, long mtype) {
    if(msgrcv(qid, msg, sizeof(Messaggio)-sizeof(long), mtype, 0) < 0) {
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