#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>
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
    Data* shared_data;
    pgid = getpid();
    struct sembuf sops;
    int semid_dir = create_sem(IPC_PRIVATE, 1);

    init_sem(semid_dir, 0, TOTAL_CHILD);

    printf("Vaa??\n");
    
    //1. Inizializzazione risorse
    int shmid = create_shm(IPC_PRIVATE, sizeof(Data));
    shared_data = (Data*)attach_shm(shmid);
    shared_data->risorse.semid = create_sem(IPC_PRIVATE, NOF_WORKERS_SEATS);
    shared_data->risorse.qid = create_queue(IPC_PRIVATE);
    printf("Era queloooo.\n");


    char semid_dir_str[8];
    snprintf(semid_dir_str, 8, "%d", semid_dir);

    char shmid_dir_str[8];
    snprintf(shmid_dir_str, 8, "%d", shmid);
    
    printf("Inizializzazione\n");

    pid_t pid;

    /*2.1 Creazione utenti*/
    char* args1[] = {"utente", semid_dir_str, shmid_dir_str, NULL};
    for(int i = 0; i < NOF_USERS; i++) {
        create_process("../bin/utente", args1);
    }

    /*2.2 Creazione operatori*/
    char* args2[] = {"operatore", semid_dir_str, shmid_dir_str, NULL};
    for(int i = 0; i < NOF_WORKERS; i++)
        create_process("../bin/operatore", args2);

    /*2.2 Creazione erogatore_ticket*/
    char* args3[] = {"erogatore", semid_dir_str, shmid_dir_str, NULL};
    create_process("../bin/erogatore", args3);
    
    //allarm(SIM_DURATION);
    srand(time(NULL));
    for(int i = 0; i < NOF_WORKERS_SEATS; i++){
        double random = (double)rand() / RAND_MAX;

        if(random > 0.25){
            shared_data->sportelli[i] = rand() % NUM_SERV + 1;
            init_sem(shared_data->risorse.semid, i, 1);
        }
        else{
            shared_data->sportelli[i] = 0;
            init_sem(shared_data->risorse.semid, i, 0);
        }
        printf("sportello %d = %d\n", i, shared_data->sportelli[i]);
    }
    
    sem_operation(sops, semid_dir, 0, 0, 0, 1);


    //kill(-pgid, SIGCONT);
    
    while(wait(NULL) > 0);

    printf("[PADRE] Tutto a posto\n");

    deleate_sem(semid_dir);
    remove_shm(shmid);

    return 0;
}