#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"
#include "ipc/message_queue.h"
#include "common.h"


int main(int argc, char* argv[]) {
    Data* datptr;
    datptr = (Data*)attach_shm(atoi(argv[1]));
    //printf("[EROGATORE] Ciao sono il figlio %d\n", getpid());
    reserve_sem(datptr->risorse.semid, 0);

    struct sembuf sops;
    sem_operation(sops, datptr->risorse.semid, 0, 0, 0, 1);

    //printf("%d\n", datptr->risorse.qid);

    detach_shm(datptr);

    printf("[EROGATORE] Ora termino\n");

    return 0;
}