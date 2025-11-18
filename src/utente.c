#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"
#include "ipc/message_queue.h"
#include "common.h"

#define N_REQUESTS 3

Data* datptr;
struct sembuf sops;
int flag_handler = 0;
sigset_t new_mask, old_mask;

void endDay_handler(int signum){
    flag_handler = 1;
}

void startDay(int qid, int sem_seat) {
    // scelta di andare

    double p_serv = (double)rand() / RAND_MAX;
    
    if(p_serv < P_SERV_MIN || p_serv > P_SERV_MAX)
        return;

    if(sigprocmask(SIG_BLOCK, &new_mask, &old_mask) < 0){
        perror("segnale non bloccato.");
        exit(EXIT_FAILURE);
    }

    struct message_t msg_snd;

    // scelta numero richieste di servizio
    
    int n_requests = rand() % N_REQUESTS + 1;

    int list_serv[n_requests];
    for(int i = 0; i < n_requests;) {
        int random = rand() % NUM_SERV + 1;

        int occorrenza = 0;
        for(int j = 0; j < i && !occorrenza; j++) {
            if(list_serv[j] == random)
                occorrenza = 1;
        }

        if(!occorrenza) {
            list_serv[i] = random;
            i++;
        }   
    }

    // scelta orario
    int ora = rand() % 10;

    long nanosec_per_hour = (long)N_NANO_SECS * ora * 60;
    struct timespec t_hour;
    t_hour.tv_sec = nanosec_per_hour / 1000000000;
    t_hour.tv_nsec = 0;

    // nanosleep
    nanosleep(&t_hour, NULL);

    // controllo esistenza servizio
    for(int i = 0; i < n_requests; i++) {
        int num_serv = list_serv[i];
        if(datptr->serv_erog[num_serv-1] > 0) {
            msg_snd.type_msg = 1;
            msg_snd.pid = getpid();
            
            snprintf(msg_snd.msg, MSG_LENGTH, "%d", num_serv);

            send_msg(qid, &msg_snd);

            struct message_t msg_rcv;
            
/*
            if(msgrcv(qid, &msg_rcv, MSG_LENGTH, getpid(), 0) < 0){
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
*/
            receive_msg(qid, &msg_rcv, getpid());

            printf("[UTENTE %d] Serv: %d, Time: %s\n", getpid(), num_serv, msg_rcv.msg);

            long nanosec_per_min = N_NANO_SECS * atol(msg_rcv.msg);
            struct timespec t_hour;
            t_hour.tv_sec = nanosec_per_min / 1000000000;
            t_hour.tv_nsec = 0;

            clock_t start, end;
            double attesa;

            start = clock();

            // nanosleep
            reserve_sem(sem_seat, num_serv);

            end = clock();

            attesa = ((double)(end - start))/CLOCKS_PER_SEC;

            printf("[UTENTE %d] Ho atteso per %f secondi\n", getpid(), attesa);

            nanosleep(&t_hour, NULL);
            
            release_sem(sem_seat, num_serv);
        }
    }

    if(sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0){
        perror("segnale non sbloccato in startDay.");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    int key;
    if((key = ftok(".", 'U')) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    int qid = create_queue(KEY_MSG);

    union semun arg;
    arg.val = 1;

    int sem_size = NUM_SERV+1;

    int semid = create_sem(key, sem_size);

    for(int i = 0; i < sem_size; i++)
        init_sem(semid, i, 1);

    datptr = (Data*)attach_shm(atoi(argv[1]));
    
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = endDay_handler;
    
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    
    sigaction(SIGUSR1, &sa, NULL);
    
    srand(time(NULL) + getpid());

    reserve_sem(datptr->risorse.semid, 0);                      //fine inizializzazione processo

    sem_operation(sops, datptr->risorse.semid, 2, 0, 0, 1);     //per iniziare giornata aspetta padre

    // decido se andare
    startDay(qid, semid);

    while(1){
        if(flag_handler) {

            //reserve_sem(sem_stat, 0);
            
            //release_sem(sem_stat, 0);

            reserve_sem(datptr->risorse.semid, 1); //segnale gestito 

            sem_operation(sops, datptr->risorse.semid, 2, 0, 0, 1); //inizio nuova giornata

            startDay(qid, semid);

            release_sem(datptr->risorse.semid, 1); //ripristino del semaforo di gestione handler

            flag_handler = 0;
        }
    }

    //delete_queue(qid);
    detach_shm(datptr);

    return 0;
}