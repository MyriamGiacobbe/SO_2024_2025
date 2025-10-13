#include <stdio.h>
#include <unistd.h>

int main() {
    printf("[UTENTE] Ciao sono il figlio %d", getpid());
    return 0;
}