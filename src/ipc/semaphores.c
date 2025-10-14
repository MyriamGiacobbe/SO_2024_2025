#include "semaphores.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int create_sem(key_t k, int nsems) {
    int semid = semget(k, nsems, IPC_CREAT | IPC_EXCL | 0600);
    if(semid == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }
    return semid;
}

void reserve_sem(int semid, int semnum) {
    struct sembuf sops = {semnum, -1, 0};
    if(semop(semid, &sops, 1) == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }
}

void release_sem(int semid, int semnum) {
    struct sembuf sops = {semnum, +1, 0};
    if(semop(semid, &sops, 1) == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }
}

void deleate_sem(int semid) {
    if(semctl(semid, 0, IPC_RMID) == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }
}
