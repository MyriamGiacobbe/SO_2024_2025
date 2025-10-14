#include <stdio.h>
#include <unistd.h>
#include "ipc/semaphores.h"

int main(int argc, char* argv[]) {
    reserve_sem(atoi(argv[1]), 0);
    printf("\n[UTENTE] Ciao sono il figlio %d\n", getpid());
    fflush(stdout);
    release_sem(atoi(argv[1]), 0);
    return 0;
}