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

void create_process(char* file_name, char* args[]) {
    pid_t pid = fork();
    if(pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if(pid == 0) {
        printf("Creo %s\n", file_name);
        setpgid(0, pgid);                   //per killarli tutti alla fine
        execvp(file_name, args);
        perror("execvp");
    }
}

int main() {
    //Data* shared_data;
    //Risorse risorse;
    pgid = getpid();
    struct sembuf sops;
    int semid_dir = create_sem(IPC_PRIVATE, 1);

    init_sem(semid_dir, 0, TOTAL_CHILD);

    /*
    //1. Inizializzazione risorse
    risorse.semid = create_sem(KEY_SEM, NOF_WORKERS_SEATS);
    risorse.qid = create_queue(KEY_MSG);
    risorse.shmid = create_shm(KEY_SHM, sizeof(Data));

    int semid = create_sem(KEY_SEM, 1);
    semctl(semid, 0, SETVAL, 0);

    char sem_str[8];
    snprintf(sem_str, 8, "%d", semid);

    char q_str[8];
    snprintf(q_str, 8, "%d", qid);

    */
    char semid_dir_str[8];
    snprintf(semid_dir_str, 8, "%d", semid_dir);

    pid_t pid;

    printf("[PADRE] Creo utenti\n");

    /*2.1 Creazione utenti*/
    char* args1[] = {"utente", semid_dir_str, NULL};
    for(int i = 0; i < NOF_USERS; i++) {
        create_process("../bin/utente", args1);
    }

    printf("[PADRE] Creo operatori\n");

    /*2.2 Creazione operatori*/
    char* args2[] = {"operatore", semid_dir_str, NULL};
    for(int i = 0; i < NOF_WORKERS; i++)
        create_process("../bin/operatore", args2);

    /*2.2 Creazione erogatore_ticket*/
    char* args3[] = {"erogatore", semid_dir_str, NULL};
    create_process("../bin/erogatore", args3);
    
    //allarm(SIM_DURATION);
    sops.sem_num = 0;
    sops.sem_flg = 0;
    sops.sem_op = 0;
    semop(semid_dir, &sops, 1);

    //kill(-pgid, SIGCONT);
    printf("Arrivo qui?? %d\n", TOTAL_CHILD);
    
    while(wait(NULL) > 0);

    printf("[PADRE] Tutto a posto\n");

    return 0;
}