#ifndef Q_MSG_H
#define Q_MSG_H

#define ERROR fprintf(stderr, \
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));
                      
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    int serv;
    pid_t pid;
} Messaggio;

int create_queue(key_t k);
void send_msg(int qid, Messaggio* msg);
void receive_msg(int qid, Messaggio* msg, long mtype);
void delete_queue(int qid);

#endif