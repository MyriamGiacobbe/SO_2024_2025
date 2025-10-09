#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "../header/common.h"
#include "../header/ipc/*.h"

int main() {
    /*1. Inizializzazione risorse */
    int semid = create_sem(KEY_SEM, N_SERV);
    int qid = create_queue(KEY_MSG);
    int shmid = create_shm(KEY_SHM, sizeof(Data), 0600);

    
    
}