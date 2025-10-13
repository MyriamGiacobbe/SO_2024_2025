#include <stdio.h>
#include <unistd.h>

int main() {
    printf("\n[EROG] Ciao sono il figlio %d\n", getpid());
    return 0;
}