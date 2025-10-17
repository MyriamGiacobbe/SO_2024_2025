#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "ipc/semaphores.h"


int main(int argc, char* argv[]) {
    printf("[UTENTE] Ciao sono il figlio %d\n", getpid());
    release_sem(atoi(argv[1]), 0);

    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_flg = 0;
    sops.sem_op = 0;
    semop(atoi(argv[1]), &sops, 1);

    printf("[UTENTE] Ora termino\n");

    return 0;
}