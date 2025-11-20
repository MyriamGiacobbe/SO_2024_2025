#include "semaphores.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int create_sem(key_t k, int nsems) {
    int semid = semget(k, nsems, IPC_CREAT | 0600);
    if(semid == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }
    return semid;
}

void init_sem(int semid, int semnum, int val) {
    if(semctl(semid, semnum, SETVAL, val) == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }
}

void sem_operation(struct sembuf sops, int semid, int semnum, int semflg, int op, int nsem) {
    sops.sem_num = semnum;
    sops.sem_flg = semflg;
    sops.sem_op = op;

    if(semop(semid, &sops, nsem) == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }    
}

int reserve_sem(int semid, int semnum) {
    struct sembuf sops = {semnum, -1, 0};
    if(semop(semid, &sops, 1) < 0) {
        if(errno == EINTR)
            return -2;
        ERROR
        exit(EXIT_FAILURE);
    }
    return 0;
}

void release_sem(int semid, int semnum) {
    struct sembuf sops = {semnum, +1, 0};
    if(semop(semid, &sops, 1) == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }
}

void delete_sem(int semid) {
    if(semctl(semid, 0, IPC_RMID) == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }
}
