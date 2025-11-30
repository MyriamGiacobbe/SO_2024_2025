#include "direttore.h"

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "common.h"
#include "ipc/message_queue.h"
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"

int giorni_sim = 1;

pid_t pgid;
Data* shared_data;
struct sembuf sops;
char semid_op_str[32];
char semid_ut_str[32];
char shmid_str[32];

const char *end_day = "day";
const char *end_sim = "sim";

int sportelli_esistenti = 0;

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
            char* args[] = {file_name, shmid_str, semid_op_str, NULL};
            execvp(file_path, args);
            perror("execvp operatore");
            exit(EXIT_FAILURE);
        }
        else if(strcmp(file_name, "utente") == 0){
            char* args[] = {file_name, shmid_str, semid_ut_str, NULL};
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
    unlink(fifo_name);
    /* Creazione FIFO per la comunicazione con add_users.c */
    int fd;
    if(mkfifo(fifo_name, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    FILE * file = fopen("../statistiche.csv", "w");
    if(file == NULL){
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    if(setpgid(0, 0) == -1) {
        perror("setpgid main");
    }
    pgid = getpid();

    srand(time(NULL) + getpid());
    //1. Inizializzazione risorse
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    int semid_seats_op = create_sem(KEY_SEM, NUM_SERV);
    int semid_seats_ut = create_sem(IPC_PRIVATE, NUM_SERV);

    for(int i = 0; i < NUM_SERV; i++)
        init_sem(semid_seats_ut, i, 1);

    int shmid = create_shm(KEY_SHM, sizeof(Data));
    shared_data = (Data*)attach_shm(shmid);

    shared_data->utenti_in_attesa = 0;
    shared_data->stat = (Statistiche){0};
    
    shared_data->semid = create_sem(IPC_PRIVATE, 4); //4 semafori: 0=Init, 1=DayEnd, 2=DayStart, 3=StatMutex
    shared_data->qid = create_queue(KEY_MSG);
    
    snprintf(shmid_str, 32, "%d", shmid);
    snprintf(semid_op_str, 32, "%d", semid_seats_op);
    snprintf(semid_ut_str, 32, "%d", semid_seats_ut);
    
    init_sem(shared_data->semid, 0, TOTAL_CHILD);       //Barriera iniziale
    init_sem(shared_data->semid, 1, WORKERS_USERS);     //Barriera fine giornata
    init_sem(shared_data->semid, 2, 1);                 //Semaforo start giornata (rosso/verde)
    init_sem(shared_data->semid, 3, 1);                 //MUTEX per le statistiche

    init_sportelli(semid_seats_op);
    
    for(int i = 0; i < NOF_USERS; i++)
        create_process("utente");

    for(int i = 0; i < NOF_WORKERS; i++)
        create_process("operatore");

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
        reserve_sem(shared_data->semid, 3);
        flag_explode = (shared_data->utenti_in_attesa > EXPLODE_THRESHOLD) ? 1 : 0;
        shared_data->utenti_in_attesa = 0;
        release_sem(shared_data->semid, 3);

        if(flag_explode)
            break;
        #endif
        
        /*Apro in scrittura la FIFO ogni giorno per avvertire add_users: 
          O_NONBLOCK serve per non far aspettare il direttore che add_users apra anche lui la fifo*/
        if((fd = open(fifo_name, O_WRONLY | O_NONBLOCK)) == -1) {
            if(errno != ENXIO)
                perror("open");
        }
        write(fd, end_day, strlen(end_day) + 1);
        close(fd);

        kill(-pgid, SIGUSR1);
        sem_operation(sops, shared_data->semid, 1, 0, 0, 1); //tutti finiscono giornata
        init_sportelli(semid_seats_op);
        leggo_stat(file);
        init_sem(shared_data->semid, 1, WORKERS_USERS);
        reserve_sem(shared_data->semid, 2);

        giorni_sim++;
    }

    if((fd = open(fifo_name, O_WRONLY | O_NONBLOCK)) == -1) {
        if(errno != ENXIO)
            perror("open");
    }
    write(fd, end_sim, strlen(end_sim) + 1);
    close(fd);

    kill(-pgid, SIGTERM);
    while(wait(NULL) > 0);
    leggo_stat(file);

    if(flag_explode)
        printf("Motivo: EXPLODE_THRESHOLD superata\n");
    else
        printf("Motivo: TIMEOUT raggiunto\n");

    delete_queue(shared_data->qid);
    delete_sem(shared_data->semid);
    delete_sem(semid_seats_op);
    delete_sem(semid_seats_ut);
    detach_shm(shared_data);
    remove_shm(shmid);

    return 0;
}

void init_sportelli(int semid){
    memset(shared_data->operatore_sportello, 0, sizeof(shared_data->operatore_sportello));
    
    int num_sportello = 0;
    int count = NOF_WORKERS_SEATS;

    for(int i = 0; i < NUM_SERV; i++){
        double random = (double)rand() / RAND_MAX;

        if(random > 0.25 && count > 0){
            int r = rand() % count + 1;
            init_sem(semid, i, r);
            shared_data->serv_erog[i] = r;
            count -= r;

            for(int j = 0; num_sportello < NOF_WORKERS_SEATS && j < r; j++) {
                shared_data->operatore_sportello[num_sportello][i] = 1;
                num_sportello++;
                sportelli_esistenti++;
            }
        }
        else{
            init_sem(semid, i, 0);
            shared_data->serv_erog[i] = 0;
            if(num_sportello < NOF_WORKERS_SEATS){
                shared_data->operatore_sportello[num_sportello][i] = 0;
                num_sportello++;
            }
        }
    }
}

void leggo_stat(FILE * file) {
    fprintf(file, "--GIORNO %d--\n", giorni_sim);

    fprintf(file, "\nSTATISTICA 1: numero di utenti serviti totali nella simulazione\n");
    for(int i = 0; i < NUM_SERV; i++)
        fprintf(file, "servizio %d: %d\n", i+1, shared_data->stat.n_utenti_serviti[i]);

    fprintf(file, "\nSTATISTICA 2: numero di utenti serviti in media al giorno\n");
    for(int i = 0; i < NUM_SERV; i++)
        fprintf(file, "servizio %d: %.2f\n", i+1, (double)shared_data->stat.n_utenti_serviti[i]/giorni_sim);

    fprintf(file, "\nSTATISTICA 3: numero di servizi erogati totali nella simulazione\n");
    for(int i = 0; i < NUM_SERV; i++)
        fprintf(file, "servizio %d: %d\n", i+1, shared_data->stat.n_serv_erog[i]);

    fprintf(file, "\nSTATISTICA 4: numero di servizi NON erogati totali nella simulazione\n");
    for(int i = 0; i < NUM_SERV; i++)
        fprintf(file, "servizio %d: %d\n", i+1, shared_data->stat.n_serv_non_erog[i]);

    fprintf(file, "\nSTATISTICA 5: numero di servizi erogati in media al giorno\n");
    for(int i = 0; i < NUM_SERV; i++)
        fprintf(file, "servizio %d: %.2f\n", i+1, (double)shared_data->stat.n_serv_erog[i]/giorni_sim);

    fprintf(file, "\nSTATISTICA 6: numero di servizi NON erogati in media al giorno\n");
    for(int i = 0; i < NUM_SERV; i++)
        fprintf(file, "servizio %d: %.2f\n", i+1, (double)shared_data->stat.n_serv_non_erog[i]/giorni_sim);

    fprintf(file, "\nSTATISTICA 7: tempo medio di attesa di utenti nella simulazione\n");
    for(int i = 0; i < NUM_SERV; i++)
        fprintf(file, "servizio %d: %f secondi\n", i+1, (double)shared_data->stat.t_attesa_utenti[i]/SIM_DURATION);

    fprintf(file, "\nSTATISTICA 8: tempo medio di attesa di utenti nella giornata\n");
    for(int i = 0; i < NUM_SERV; i++)
        fprintf(file, "servizio %d: %f secondi\n", i+1, (double)shared_data->stat.t_attesa_utenti[i]/giorni_sim);

    double tempo_medio[NUM_SERV];
    for(int i = 0; i < NUM_SERV; i++) {
        if(shared_data->stat.t_erog_serv[i] == 0 || shared_data->stat.n_serv_erog[i] == 0)
            tempo_medio[i] = 0;
        else
            tempo_medio[i] += (double)shared_data->stat.t_erog_serv[i]/shared_data->stat.n_serv_erog[i];
    }

    fprintf(file, "\nSTATISTICA 9: tempo medio di erogazione dei servizi nella simulazione\n");
    for(int i = 0; i < NUM_SERV; i++) {
        if(tempo_medio[i] <= 0)
            fprintf(file, "servizio %d: %d minuti\n", i+1, 0);
        else
            fprintf(file, "servizio %d: %.1f minuti\n", i+1, tempo_medio[i]/giorni_sim);
    }

    fprintf(file, "\nSTATISTICA 10: tempo medio di erogazione dei servizi nella giornata\n");
    for(int i = 0; i < NUM_SERV; i++)
        fprintf(file, "servizio %d: %.1f minuti\n", i+1, tempo_medio[i]);

    fprintf(file, "\nSTATISTICA 11: numero di operatori attivi durante la giornata\n");
    fprintf(file, "%d\n", shared_data->stat.n_op_attivi_giorno);

    shared_data->stat.n_op_attivi_giorno = 0;

    fprintf(file, "\nSTATISTICA 12: numero di operatori attivi durante la simulazione:\n");
    fprintf(file, "%d\n", shared_data->stat.n_op_attivi_sim);

    fprintf(file, "\nSTATISTICA 13: numero medio di pause effettuate nella giornata\n");
    fprintf(file, "%.2f\n", (double)shared_data->stat.n_pause_sim/giorni_sim);

    fprintf(file, "\nSTATISTICA 14: numero totale di pause effettuate nella simulazione\n");
    fprintf(file, "%d\n", shared_data->stat.n_pause_sim);

    fprintf(file, "\nSTATISTICA 15: rapporto tra operatori disponibili e sportelli esistenti\n");
    for(int i = 0; i < NOF_WORKERS_SEATS; i++) {
        fprintf(file, "operatori / sportello %d: %.2f\n", i+1, (double)shared_data->stat.operatore_sportello_giorno[i]/sportelli_esistenti);
        shared_data->stat.operatore_sportello_giorno[i] = 0;
    }
    fprintf(file, "\n");
}