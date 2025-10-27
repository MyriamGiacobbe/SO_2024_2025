#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"
#include "ipc/message_queue.h"
#include "common.h"


int main(int argc, char* argv[]) {
    //printf("[UTENTE] Ciao sono il figlio %d\n", getpid());
    reserve_sem(atoi(argv[1]), 0);

    struct sembuf sops;
    sem_operation(sops, atoi(argv[1]), 0, 0, 0, 1);

    Data* datptr;
    datptr = (Data*)attach_shm(atoi(argv[2]));
    //printf("%d\n", datptr->risorse.qid);

    detach_shm(datptr);

    printf("[UTENTE] Ora termino\n");

    return 0;
}