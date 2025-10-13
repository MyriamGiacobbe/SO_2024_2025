#include <stdio.h>
#include <unistd.h>

int main() {
    printf("[EROG] Ciao sono il figlio %d", getpid());
    return 0;
}