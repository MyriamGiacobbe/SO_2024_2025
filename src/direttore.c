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

void init_seats(int semid){
    srand(time(NULL));

    
    int count = NOF_WORKERS_SEATS;
    for(int i = 0; i < NUM_SERV; i++){
        double random = (double)rand() / RAND_MAX;

        if(random > 0.25 && count > 0){
            int r = rand() % count + 1;
            init_sem(semid, i, r);
            count -= r;
        }
        else{
            init_sem(semid, i, 0);
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
    
    int semid_seats = create_sem(IPC_PRIVATE, NUM_SERV);

    //1. Inizializzazione risorse
    int shmid = create_shm(IPC_PRIVATE, sizeof(Data));
    shared_data = (Data*)attach_shm(shmid);
    
    shared_data->risorse.semid = create_sem(IPC_PRIVATE, 3);
    shared_data->risorse.qid = create_queue(IPC_PRIVATE);
    
    char shmid_dir_str[8];
    snprintf(shmid_dir_str, 8, "%d", shmid);
    
    init_sem(shared_data->risorse.semid, 0, TOTAL_CHILD+1);
    init_sem(shared_data->risorse.semid, 1, NOF_WORKERS);
    init_sem(shared_data->risorse.semid, 2, 1);

    init_seats(semid_seats);

    
    char semid_str[8];
    snprintf(semid_str, 8, "%d", semid_seats);
    
    pid_t pid;
    
    /*2.1 Creazione utenti*/
    char* args1[] = {"utente", shmid_dir_str, NULL};
    for(int i = 0; i < NOF_USERS; i++) {
        create_process("../bin/utente", args1);
    }
    
    /*2.2 Creazione operatori*/
    char* args2[] = {"operatore", shmid_dir_str, semid_str, NULL};
    for(int i = 0; i < NOF_WORKERS; i++)
    create_process("../bin/operatore", args2);

    /*2.2 Creazione erogatore_ticket*/
    char* args3[] = {"erogatore", shmid_dir_str, NULL};
    create_process("../bin/erogatore", args3);

    //allarm(SIM_DURATION);

    sem_operation(sops, shared_data->risorse.semid, 0, 0, 1, 1); //waitforzero -> aspetta che tutti i processi siano pronti

    
    struct timespec t_day;
    t_day.tv_sec = 1;
    t_day.tv_nsec = N_NANO_SECS;
    
    alarm(SIM_DURATION*60*24);
    
    reserve_sem(shared_data->risorse.semid, 0);

    //while(1){
    if(nanosleep(&t_day, NULL) == 0){
        kill(-pgid, SIGUSR1);
        //rinizializzare sportelli
        sem_operation(sops, shared_data->risorse.semid, 1, 0, 0, 1); //waitforzero -> aspetta che gli operatori gestiscono il segnale
        printf("Leggo le statistiche\n");
        init_seats(semid_seats);
        reserve_sem(shared_data->risorse.semid, 2);
    }  
        //break;
//}
    
    

    printf("[PADRE] Tutto a posto\n");

    delete_sem(shared_data->risorse.semid);
    delete_sem(semid_seats);
    remove_shm(shmid);

    return 0;
}
