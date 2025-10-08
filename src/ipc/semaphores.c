#include "../header/ipc/semaphores.h"
#include "../header/common.h"

#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

int create_sem(key_t k, int senum) {
    int semid = semget(k, senum, IPC_CREAT | IPC_EXCL | 0600);
    if(semid == -1) {

    }
}

int reserve_sem(int semid, int semnum);
int release_sem(int semid, int semnum);
void deleate_sem(int semid);
