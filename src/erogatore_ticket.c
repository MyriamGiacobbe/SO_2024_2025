#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "ipc/semaphores.h"


int main(int argc, char* argv[]) {
    printf("[EROGATORE] Ciao sono il figlio %d\n", getpid());
    reserve_sem(atoi(argv[1]), 0);

    struct sembuf sops;
    sem_operation(sops, atoi(argv[1]), 0, 0, 0, 1);

    printf("[EROGATORE] Ora termino\n");

    return 0;
}