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


#define TOTAL_CHILD NOF_WORKERS + NOF_USERS + 1

pid_t pgid;
Data* shared_data;
struct sembuf sops;

void leggo_stat() {
    printf("Operatori attivi durante il giorno: %d\n", shared_data->stat.n_op_attivi_giorno);
    printf("Operatori attivi durante la simulazione: %d\n", shared_data->stat.n_op_attivi_sim);
    printf("Pause effettuate durante il giorno: %d\n", shared_data->stat.n_pause_giorno);
    printf("Pause effettuate durante la simulazione: %d\n", shared_data->stat.n_pause_sim);
}

void init_seats(int semid){
    //printf("[DEBUG - DIRETTORE] Sportelli rinizializzati\n");
    
    srand(time(NULL));
    
    int count = NOF_WORKERS_SEATS;
    for(int i = 0; i < NUM_SERV; i++){
        double random = (double)rand() / RAND_MAX;

        if(random > 0.25 && count > 0){
            int r = rand() % count + 1;
            init_sem(semid, i, SETVAL, r);

            shared_data->serv_erog[i] = r;

            count -= r;
        }
        else{
            init_sem(semid, i, SETVAL, 0);

            shared_data->serv_erog[i] = 0;
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

    Statistiche stat = {0}; 

    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &sa, NULL);
    
    int semid_seats = create_sem(IPC_PRIVATE, NUM_SERV);

    //1. Inizializzazione risorse
    int shmid = create_shm(IPC_PRIVATE, sizeof(Data));
    shared_data = (Data*)attach_shm(shmid);

    shared_data->stat = stat;
    
    shared_data->risorse.semid = create_sem(IPC_PRIVATE, 3);
    shared_data->risorse.qid = create_queue(IPC_PRIVATE);
    
    char shmid_dir_str[8];
    snprintf(shmid_dir_str, 8, "%d", shmid);
    
    init_sem(shared_data->risorse.semid, 0, SETVAL, TOTAL_CHILD);   //tutti pronti
    init_sem(shared_data->risorse.semid, 1, SETVAL, NOF_WORKERS);   //tutti hanno concluso giornata
    init_sem(shared_data->risorse.semid, 2, SETVAL, 1);             //per iniziare nuova giornata

    init_seats(semid_seats);

    
    char semid_str[8];
    snprintf(semid_str, 8, "%d", semid_seats);
    
    //pid_t pid;
    
    /* 2.1 Creazione utenti */
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

    sem_operation(sops, shared_data->risorse.semid, 0, 0, 0, 1); //waitforzero -> aspetta che tutti i processi siano pronti
    
    long nanosec_per_day = (long)N_NANO_SECS * 24 * 60; // 1440 * 8.000.000
    struct timespec t_day;
    t_day.tv_sec = nanosec_per_day / 1000000000;      // Risultato: 11
    t_day.tv_nsec = nanosec_per_day % 1000000000;     // Risultato: 520000000    

    //alarm(((double)nanosec_per_day/1000000000)*SIM_DURATION);
    
    reserve_sem(shared_data->risorse.semid, 2);  //tutti iniziano giornata

    int count = 0;
    while(count < SIM_DURATION){
        nanosleep(&t_day, NULL);

        release_sem(shared_data->risorse.semid, 2);  //ripristinare flag di inizio giornata

        kill(-pgid, SIGUSR1);

        sem_operation(sops, shared_data->risorse.semid, 1, 0, 0, 1); //tutti finiscono giornata

        init_seats(semid_seats);

        leggo_stat();
        
        reserve_sem(shared_data->risorse.semid, 2);

        count++;
    }

    kill(-pgid, SIGTERM);

    printf("[PADRE] Tutto a posto\n");

    delete_sem(shared_data->risorse.semid);
    delete_sem(semid_seats);
    remove_shm(shmid);

    return 0;
}
