#include <stdio.h>
#include <unistd.h>

int main() {
    printf("\n[OPERATORE] Ciao sono il figlio %d\n", getpid());
    return 0;
}