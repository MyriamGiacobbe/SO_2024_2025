#ifndef SEM_H
#define SEM_H

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
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
};

int create_sem(key_t k, int senum);
void init_sem(int semid, int semnum, int val);
void sem_operation(struct sembuf sops, int semid, int semnum, int semflg, int op, int nsem);
void reserve_sem(int semid, int semnum);
void release_sem(int semid, int semnum);
void delete_sem(int semid);

#endif