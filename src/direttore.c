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
Data* shared_data;
struct sembuf sops;

void init_seats(){
    srand(time(NULL));

    
    int count = NOF_WORKERS_SEATS;
    for(int i = 0; i < NUM_SERV; i++){
        double random = (double)rand() / RAND_MAX;

        if(random > 0.25 && count > 0){
            int r = rand() % count + 1;
            init_sem(shared_data->risorse.semid, i, r);
            count -= r;
        }
        else{
            init_sem(shared_data->risorse.semid, i, 0);
        }
    }
}

void create_process(char* file_name, char* args[]) {
    pid_t pid = fork();
    if(pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if(pid == 0) {
        setpgid(0, pgid);                   //per killarli tutti alla fine
        execvp(file_name, args);
        perror("execvp");
    }
}

int main() {
    pgid = getpid();
    
    int semid_dir = create_sem(IPC_PRIVATE, 1);

    init_sem(semid_dir, 0, TOTAL_CHILD);
    
    //1. Inizializzazione risorse
    int shmid = create_shm(IPC_PRIVATE, sizeof(Data));
    shared_data = (Data*)attach_shm(shmid);
    shared_data->risorse.semid = create_sem(IPC_PRIVATE, NUM_SERV);
    shared_data->risorse.qid = create_queue(IPC_PRIVATE);

    char semid_dir_str[8];
    snprintf(semid_dir_str, 8, "%d", semid_dir);

    char shmid_dir_str[8];
    snprintf(shmid_dir_str, 8, "%d", shmid);

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
    
    init_seats();

    sem_operation(sops, semid_dir, 0, 0, 0, 1); //waitforzero -> aspetta che tutti i processi siano pronti

    init_sem(semid_dir, 0, NOF_WORKERS);

    struct timespec t_day;
    t_day.tv_sec = 1;
    t_day.tv_nsec = N_NANO_SECS;

    alarm(SIM_DURATION*60*24);
    
    while(1){
        if(nanosleep(&t_day, NULL) == 0){
            kill(-pgid, SIGUSR1);
            //rinizializzare sportelli
            sem_operation(sops, semid_dir, 0, 0, 0, 1); //waitforzero -> aspetta che gli operatori gestiscono il segnale
            printf("Leggo le statistiche\n");
            init_seats();
            release_sem(semid_dir, 0);
        }  
        //break;
    }
    
    

    printf("[PADRE] Tutto a posto\n");

    delete_sem(semid_dir);
    remove_shm(shmid);

    return 0;
}
