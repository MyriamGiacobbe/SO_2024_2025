#include <stdio.h>
#include <unistd.h>

int main() {
    printf("\n[UTENTE] Ciao sono il figlio %d\n", getpid());
    return 0;
}