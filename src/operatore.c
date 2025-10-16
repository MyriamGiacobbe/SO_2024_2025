#include <stdio.h>
#include <unistd.h>
#include "ipc/semaphores.h"

int main(int argc, char* argv[]) {
    raise(SIGSTOP);
    printf("[OPERATORE] Ciao sono il figlio %d\n", getpid());
    return 0;
}