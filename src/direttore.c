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

#define TOTAL_CHILD NOF_USERS+NOF_WORKERS+1

pid_t pgid;
int count = 0;

void create_process(char* file_name, char* args[]) {
    pid_t pid = fork();
    if(pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if(pid == 0) {
        printf("Creo %S\n", file_name);
        count++;
        setpgid(0, pgid);
        execvp(file_name, args);
    }
}

int main() {
    //Data* shared_data;
    //Risorse risorse;
    pgid = getpid();

    /*
    //1. Inizializzazione risorse
    risorse.semid = create_sem(KEY_SEM, NOF_WORKERS_SEATS);
    risorse.qid = create_queue(KEY_MSG);
    risorse.shmid = create_shm(KEY_SHM, sizeof(Data));

    //setgpid(0, getpid());

    int semid = create_sem(KEY_SEM, 1);
    semctl(semid, 0, SETVAL, 0);

    char sem_str[8];
    snprintf(sem_str, 8, "%d", semid);

    char q_str[8];
    snprintf(q_str, 8, "%d", qid);

    char shm_str[8];
    snprintf(shm_str, 8, "%d", shmid);
    */

    pid_t pid;

    printf("[PADRE] Creo utenti\n");

    /*2.1 Creazione utenti*/
    for(int i = 0; i < NOF_USERS; i++)
        create_process("../bin/utente", NULL);

    printf("[PADRE] Creo operatori\n");

    /*2.2 Creazione operatori*/
    for(int i = 0; i < NOF_WORKERS; i++)
        create_process("../bin/operatore", NULL);

    /*2.2 Creazione erogatore_ticket*/
    create_process("../bin/erogatore", NULL);
    
    //allarm(SIM_DURATION);

    while(count < TOTAL_CHILD);
    kill(-pgid, SIGCONT);
    
    
    while(wait(NULL) > 0);

    printf("[PADRE] Tutto a posto\n");

    return 0;
}