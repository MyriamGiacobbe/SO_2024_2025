#include <stdio.h>
#include <unistd.h>

int main() {
    printf("[OPERATORE] Ciao sono il figlio %d", getpid());
    return 0;
}