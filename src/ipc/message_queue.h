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

#define KEY_MSG_UE 2025
#define KEY_MSG_UO 2026

#define MSG_LENGTH 120

struct message_t{
    long type_msg;
    pid_t pid;
    char msg[MSG_LENGTH];
};

int create_queue(key_t k);
void send_msg(int qid, struct message_t *msg);
void receive_msg(int qid, struct message_t *msg, long mtype);
void delete_queue(int qid);

#endif