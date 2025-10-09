#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "common.h"
#include "ipc/message_queue.h"
#include "ipc/semaphores.h"
#include "ipc/shared_mmemory.h"
#include "ipc/signals.h"

int main() {
    /*1. Inizializzazione risorse */
    int semid = create_sem(KEY_SEM, NOF_WORKERS_SEATS);
    int qid = create_queue(KEY_MSG);
    int shmid = create_shm(KEY_SHM, sizeof(Data), 0600);

    /*2.1 Creazione utenti*/
    for(int i = 0; i < NOF_WORKERS; i++) {
        pid_t pid = fork();
        switch(pid) {
            case -1:
                ERROR
                exit(EXIT_FAILURE);
            case 0:
                char* args[] = {"utente", NULL};
                execvp("../bin/utente", args);
        }
    }
    
}