#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "common.h"
#include "ipc/message_queue.h"
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"
//#include "ipc/signals.h"

int main() {
    /*
    Data* shared_data;
    Risorse risorse;

    //1. Inizializzazione risorse
    risorse.semid = create_sem(KEY_SEM, NOF_WORKERS_SEATS);
    risorse.qid = create_queue(KEY_MSG);
    risorse.shmid = create_shm(KEY_SHM, sizeof(Data));
    */

    setpgid(0, 0);

    int semid = create_sem(KEY_SEM, 1);
    semctl(semid, 0, SETVAL, 0);

    char str[10];
    snprintf(str, 10, "%d", semid);

    pid_t pid;

    printf("[PADRE] Creo utenti\n");

    /*2.1 Creazione utenti*/
    for(int i = 0; i < NOF_USERS; i++) {
        pid = fork();
        switch(pid) {
            case -1:
                ERROR
                exit(EXIT_FAILURE);
            case 0:
                setpgid(0, getppid());
                printf("[UTENTE %d] Creato\n", getpid());
                char* args[] = {"utente", str, NULL};
                execvp("../bin/utente", args);
            default:
        }
    }

    printf("[PADRE] Creo operatori\n");

    /*2.2 Creazione operatori*/
    for(int i = 0; i < NOF_WORKERS; i++) {
        pid = fork();
        switch(pid) {
            case -1:
                ERROR
                exit(EXIT_FAILURE);
            case 0:
                setpgid(0, getppid());
                printf("[OPERATORE %d] Creato\n", getpid());
                char* args[] = {"operatore", str, NULL};
                execvp("../bin/operatore", args);
            default:
        }
    }

    printf("[PADRE] Creo erogatore\n");
    /*2.3 Creazione erogatore*/
    pid = fork();
    switch(pid) {
        case -1:
            ERROR
            exit(EXIT_FAILURE);
        case 0:
            setpgid(0, getppid());
            printf("[EROG %d] Creato\n", getpid());
            char* args[] = {"erogatore", str, NULL};
            execvp("../bin/erogatore", args);
        default:
    }
    
    //allarm(SIM_DURATION);

    
    for(int i = 0; i < (NOF_USERS+NOF_WORKERS+1); i++) {
        //printf("[PADRE] Non entro nel for\n");
        release_sem(semid, 0);
    }
    
    while(wait(NULL) > 0);

    deleate_sem(semid);

    printf("[PADRE] Tutto a posto\n");

    return 0;
}