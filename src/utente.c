#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "ipc/semaphores.h"


int main(int argc, char* argv[]) {
    raise(SIGSTOP);
    printf("[UTENTE] Ciao sono il figlio %d\n", getpid());
    return 0;
}