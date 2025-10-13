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

int stopped = 0;

void handle_signal(int signum) {
    int status;
    pid_t pid;
    while((pid = waitpid(-1, &status, WUNTRACED)) > 0) {
        if(WIFSTOPPED(status))
            stopped++;
    }
}

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

    struct sigaction sa;
    bzero(&sa, sizeof(sa)); //setta tutti i bytes a 0
    sa.sa_handler = handle_signal; //puntatore alla funzione handle_signal
    sigaction(SIGCHLD, &sa, NULL); //setta l'handler nuovo

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
                printf("[UTENTE %d] Creato\n", getpid());
                setpgid(0, getppid());
                raise(SIGSTOP);
                char* args[] = {"utente", NULL};
                execvp("../bin/utente", args);
                exit(EXIT_SUCCESS);
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
                printf("[OPERATORE %d] Creato\n", getpid());
                setpgid(0, getppid());
                raise(SIGSTOP);
                char* args[] = {"opertaore", NULL};
                execvp("../bin/opertaore", args);
                exit(EXIT_SUCCESS);
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
            printf("[EROG %d] Creato\n", getpid());
            setpgid(0, getppid());
            raise(SIGSTOP);
            char* args[] = {"opertaore", NULL};
            execvp("../bin/opertaore", args);
            exit(EXIT_SUCCESS);
        default:
    }
    
    //allarm(SIM_DURATION);

    while(stopped < (NOF_USERS + NOF_WORKERS + 1))
        pause();

    kill(-getpid(), SIGCONT);

    for(int i = 0; i < (NOF_USERS + NOF_WORKERS + 1); i++)
        wait(NULL);

    printf("[PADRE] Tutto a posto\n");

    return 0;
}