#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "common.h"
#include "ipc/message_queue.h"
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"
#include "ipc/signals.h"

int main() {
    Data* shared_data;
    Risorse risorse;

    /*1. Inizializzazione risorse */
    risorse.semid = create_sem(KEY_SEM, NOF_WORKERS_SEATS);
    risorse.qid = create_queue(KEY_MSG);
    risorse.shmid = create_shm(KEY_SHM, sizeof(Data));

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