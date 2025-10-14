#include <stdio.h>
#include <unistd.h>
#include "ipc/semaphores.h"

int main(int argc, char* argv[]) {
    reserve_sem(atoi(argv[1]), 0);
    printf("\n[OPERATORE] Ciao sono il figlio %d\n", getpid());
    release_sem(atoi(argv[1]), 0);
    return 0;
}