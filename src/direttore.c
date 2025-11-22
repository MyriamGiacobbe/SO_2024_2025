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
#define WORKERS_USERS NOF_WORKERS + NOF_USERS

int giorni_sim = 1;

pid_t pgid;
Data* shared_data;
struct sembuf sops;
char semid_str[32];
char shmid_str[32];

int sportelli_esistenti = 0;

void leggo_stat() {
    printf("Operatori attivi durante il giorno: %d\n", shared_data->stat.n_op_attivi_giorno);

    shared_data->stat.n_op_attivi_giorno = 0;

    printf("Operatori attivi durante la simulazione: %d\n", shared_data->stat.n_op_attivi_sim);

    printf("Numero di pause in media effettuate durante il giorno: %.2f\n", (double)shared_data->stat.n_pause_sim/giorni_sim);

    printf("Pause effettuate durante la simulazione: %d\n\n", shared_data->stat.n_pause_sim);

    printf("Numero di utenti serviti durante la simulazione:\n");
    for(int i = 0; i < NUM_SERV; i++)
        printf("servizio %d: %d\n", i+1, shared_data->stat.n_utenti_serviti[i]);

    printf("\nNumero di utenti serviti in medio al giorno:\n");
    for(int i = 0; i < NUM_SERV; i++)
        printf("servizio %d: %.2f\n", i+1, (double)shared_data->stat.n_utenti_serviti[i]/giorni_sim);

    printf("\nTempo medio di attesa di utenti durante simulazione:\n");
    for(int i = 0; i < NUM_SERV; i++)
        printf("servizio %d: %f secondi\n", i+1, (double)shared_data->stat.t_attesa_utenti[i]/SIM_DURATION);

    printf("\nTempo medio di attesa di utenti al giorno:\n");
    for(int i = 0; i < NUM_SERV; i++)
        printf("servizio %d: %f secondi\n", i+1, (double)shared_data->stat.t_attesa_utenti[i]/giorni_sim);

    printf("\nNumero di servizi erogati durante la simulazione:\n");
    for(int i = 0; i < NUM_SERV; i++)
        printf("servizio %d: %d\n", i+1, shared_data->stat.n_serv_erog[i]);

    printf("\nNumero di servizi NON erogati durante la simulazione:\n");
    for(int i = 0; i < NUM_SERV; i++)
        printf("servizio %d: %d\n", i+1, shared_data->stat.n_serv_non_erog[i]);

    printf("\nNumero di servizi erogati in media al giorno:\n");
    for(int i = 0; i < NUM_SERV; i++)
        printf("servizio %d: %.2f\n", i+1, (double)shared_data->stat.n_serv_erog[i]/giorni_sim);

    printf("\nNumero di servizi NON erogati in media al giorno:\n");
    for(int i = 0; i < NUM_SERV; i++)
        printf("servizio %d: %.2f\n", i+1, (double)shared_data->stat.n_serv_non_erog[i]/giorni_sim);

    double tempo_medio[NUM_SERV];
    for(int i = 0; i < NUM_SERV; i++) {
        if(shared_data->stat.t_erog_serv[i] == 0 || shared_data->stat.n_serv_erog[i] == 0)
            tempo_medio[i] = 0;
        else
            tempo_medio[i] += (double)shared_data->stat.t_erog_serv[i]/shared_data->stat.n_serv_erog[i];
    }

    printf("\nTempo medio di erogazione servizi nella simulazione:\n");
    for(int i = 0; i < NUM_SERV; i++) {
        if(tempo_medio[i] <= 0)
            printf("servizio %d: %d minuti\n", i+1, 0);
        else
            printf("servizio %d: %.1f minuti\n", i+1, tempo_medio[i]/giorni_sim);
    }

    printf("\nTempo medio di erogazione servizi al giorno:\n");
    for(int i = 0; i < NUM_SERV; i++)
        printf("servizio %d: %.1f minuti\n", i+1, tempo_medio[i]);

    printf("\nRaporto tra operatori disponibili e sportelli esistenti per ogni sportello per ogni giorno:\n");
    for(int i = 0; i < NOF_WORKERS_SEATS; i++) {
        printf("operatori / sportello %d: %.2f\n", i+1, (double)shared_data->stat.operatore_sportello_giorno[i]/sportelli_esistenti);
        shared_data->stat.operatore_sportello_giorno[i] = 0;
    }
    
}

void init_seats(int semid){
    srand(time(NULL) + getpid());

    int num_sportello = 0;

    int count = NOF_WORKERS_SEATS;
    for(int i = 0; i < NUM_SERV; i++){
        double random = (double)rand() / RAND_MAX;

        printf("[DIR] init_seats: count = %d\n", count);

        if(random > 0.25 && count > 0){
            int r = rand() % count + 1;
            printf("[DIR] init_seats: r = %d\n", r);
            init_sem(semid, i, r);

            shared_data->serv_erog[i] = r;

            count -= r;

            printf("[DIR] init_seats: count -= r => %d\n", count);

            for(int j = 0; num_sportello < NOF_WORKERS_SEATS && j < r; j++) {
                shared_data->operatore_sportello[num_sportello][i] = 1;
                num_sportello++;
                sportelli_esistenti++;
            }

            printf("[DIR] init_seats: num_sportello = %d\n", num_sportello);
        }
        else{
            init_sem(semid, i, 0);

            shared_data->serv_erog[i] = 0;

            shared_data->operatore_sportello[num_sportello][i] = 0;
            num_sportello++;
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

        if(setpgid(0, pgid) < 0) { //per killarli tutti alla fine
            perror("setpgid padre");
            exit(EXIT_FAILURE);
        }

        if(strcmp(file_name, "operatore") == 0){
            char* args[] = {file_name, shmid_str, semid_str, NULL};
            execvp(file_path, args);
            perror("execvp operatore");
            exit(EXIT_FAILURE);
        }
        else{
            char* args[] = {file_name, shmid_str, NULL};
            execvp(file_path, args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
}

int main() {

    setbuf(stdout, NULL);

    key_t key;
    if((key = ftok(".", 'D')) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    if(setpgid(0, 0) == -1) {
        perror("setpgid main");
    }
    pgid = getpid();

    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    int semid_seats = create_sem(key, NUM_SERV);

    //1. Inizializzazione risorse
    int shmid = create_shm(IPC_PRIVATE, sizeof(Data));
    shared_data = (Data*)attach_shm(shmid);

    shared_data->utenti_in_attesa = 0;
    shared_data->stat = (Statistiche){0};
    
    shared_data->semid = create_sem(IPC_PRIVATE, 4); //4 semafori: 0=Init, 1=DayEnd, 2=DayStart, 3=StatMutex

    shared_data->qid = create_queue(KEY_MSG);
    
    snprintf(shmid_str, 32, "%d", shmid);
    snprintf(semid_str, 32, "%d", semid_seats);
    
    init_sem(shared_data->semid, 0, TOTAL_CHILD);       //Barriera iniziale
    init_sem(shared_data->semid, 1, WORKERS_USERS);     //Barriera fine giornata
    init_sem(shared_data->semid, 2, 1);                 //Semaforo start giornata (rosso/verde)
    init_sem(shared_data->semid, 3, 1);                 //MUTEX per le statistiche

    init_seats(semid_seats);
    
    /* 2.1 Creazione utenti*/
    for(int i = 0; i < NOF_USERS; i++) {
        create_process("utente");
    }

    /*2.2 Creazione operatori*/
    for(int i = 0; i < NOF_WORKERS; i++)
        create_process("operatore");

    /*2.2 Creazione erogatore_ticket*/
    create_process("erogatore");

    sem_operation(sops, shared_data->semid, 0, 0, 0, 1); //waitforzero -> aspetta che tutti i processi siano pronti
    
    long nanosec_per_day = (long)N_NANO_SECS * 10 * 60;
    struct timespec t_day;
    t_day.tv_sec = nanosec_per_day / 1000000000;
    t_day.tv_nsec = nanosec_per_day % 1000000000;
    
    reserve_sem(shared_data->semid, 2);  //tutti iniziano giornata

    int flag_explode = 0;

    while(giorni_sim < SIM_DURATION){
        nanosleep(&t_day, NULL);

        release_sem(shared_data->semid, 2);  //ripristinare flag di inizio giornata

        #ifdef EXPLODE
        if(shared_data->utenti_in_attesa > EXPLODE_THRESHOLD) {
            flag_explode = 1;
            break;
        }
        #endif

        kill(-pgid, SIGUSR1);

        sem_operation(sops, shared_data->semid, 1, 0, 0, 1); //tutti finiscono giornata

        init_seats(semid_seats);

        printf("\n");
        leggo_stat();
        printf("\n");

        init_sem(shared_data->semid, 1, WORKERS_USERS);

        reserve_sem(shared_data->semid, 2);

        giorni_sim++;
    }

    kill(-pgid, SIGTERM);

    sem_operation(sops, shared_data->semid, 1, 0, 0, 1);

    leggo_stat();

    if(flag_explode)
        printf("Motivo: EXPLODE_THRESHOLD superata\n");
    else
        printf("Motivo: TIMEOUT raggiunto\n");

    delete_queue(shared_data->qid);
    delete_sem(shared_data->semid);
    delete_sem(semid_seats);
    detach_shm(shared_data);
    remove_shm(shmid);

    return 0;
}