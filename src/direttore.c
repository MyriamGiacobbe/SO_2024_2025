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
char semid_str[8];
char shmid_dir_str[8];

void leggo_stat() {
    printf("Operatori attivi durante il giorno: %d\n", shared_data->stat.n_op_attivi_giorno);
    printf("Operatori attivi durante la simulazione: %d\n", shared_data->stat.n_op_attivi_sim);
    printf("Pause effettuate durante il giorno: %d\n", shared_data->stat.n_pause_giorno);
    printf("Pause effettuate durante la simulazione: %d\n\n", shared_data->stat.n_pause_sim);
}

void init_seats(int semid){
    srand(time(NULL));
    
    int count = NOF_WORKERS_SEATS;
    for(int i = 0; i < NUM_SERV; i++){
        double random = (double)rand() / RAND_MAX;

        if(random > 0.25 && count > 0){
            int r = rand() % count + 1;
            init_sem(semid, i, r);

            shared_data->serv_erog[i] = r;

            count -= r;
        }
        else{
            init_sem(semid, i, 0);

            shared_data->serv_erog[i] = 0;
        }
    }
}

void create_process(char* file_name) {
    char file_path[128];
    snprintf(file_path, 128, "../bin/%s", file_name);
    
    pid_t pid = fork();
    if(pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if(pid == 0) {
        setpgid(0, pgid);                   //per killarli tutti alla fine
        if(strcmp(file_name, "operatore") == 0){
            char* args[] = {file_name, shmid_dir_str, semid_str, NULL};
            execvp(file_path, args);
            perror("execvp operatore");
            exit(EXIT_FAILURE);
        }
        else{
            char* args[] = {file_name, shmid_dir_str, NULL};
            execvp(file_path, args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
}

int main() {
    key_t key;
    if((key = ftok(".", 'D')) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    pgid = getpid();

    Statistiche stat = {0}; 

    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &sa, NULL);
    
    int semid_seats = create_sem(key, NUM_SERV);

    //1. Inizializzazione risorse
    int shmid = create_shm(IPC_PRIVATE, sizeof(Data));
    shared_data = (Data*)attach_shm(shmid);

    shared_data->stat = stat;
    
    shared_data->semid = create_sem(key, 3);
    
    snprintf(shmid_dir_str, 8, "%d", shmid);
    
    init_sem(shared_data->semid, 0, TOTAL_CHILD);   //tutti pronti
    init_sem(shared_data->semid, 1, NOF_WORKERS+NOF_USERS);   //tutti hanno concluso giornata
    init_sem(shared_data->semid, 2, 1);             //per iniziare nuova giornata

    init_seats(semid_seats);
    
    snprintf(semid_str, 8, "%d", semid_seats);
    
    /* 2.1 Creazione utenti */
    for(int i = 0; i < NOF_USERS; i++) {
        create_process("utente");
    }
    
    /*2.2 Creazione operatori*/
    for(int i = 0; i < NOF_WORKERS; i++)
        create_process("operatore");

    /*2.2 Creazione erogatore_ticket*/
    create_process("erogatore");

    sem_operation(sops, shared_data->semid, 0, 0, 0, 1); //waitforzero -> aspetta che tutti i processi siano pronti

    printf("[direttore] Il bagno è libero e ora papà può andare a cagare\n");
    
    long nanosec_per_day = (long)N_NANO_SECS * 10 * 60; // 1440 * 8.000.000
    struct timespec t_day;
    t_day.tv_sec = nanosec_per_day / 1000000000;      // Risultato: 11
    t_day.tv_nsec = nanosec_per_day % 1000000000;     // Risultato: 520000000    
    
    reserve_sem(shared_data->semid, 2);  //tutti iniziano giornata

    int count = 0;
    while(count < SIM_DURATION){
        nanosleep(&t_day, NULL);

        release_sem(shared_data->semid, 2);  //ripristinare flag di inizio giornata

        kill(-pgid, SIGUSR1);

        sem_operation(sops, shared_data->semid, 1, 0, 0, 1); //tutti finiscono giornata

        init_seats(semid_seats);

        leggo_stat();
        
        reserve_sem(shared_data->semid, 2);

        count++;
    }

    kill(-pgid, SIGTERM);

    delete_sem(shared_data->semid);
    delete_sem(semid_seats);
    detach_shm(shared_data);
    remove_shm(shmid);

    return 0;
}
