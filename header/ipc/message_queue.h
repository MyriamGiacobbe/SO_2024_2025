#ifndef Q_MSG_H
#define Q_MSG_H

typedef struct {
    int serv;
    pid_t pid;
} Messaggio;

int create_queue(key_t k);
void send_msg(int qid, Messaggio* msg);
void receive_msg(int qid, Messaggio* msg, long mtype);
void delete_queue(int qid);

#endif