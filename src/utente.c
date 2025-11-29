#include "utente.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"
#include "ipc/message_queue.h"
#include "common.h"

#define N_REQUESTS 3

int n_utenti_serv = 0, t_attesa = 0;
Data* datptr;
struct sembuf sops;
int flag_endDay = 0, flag_endSim = 0;
sigset_t new_mask, old_mask;
int num_serv = 0;

void signal_handler(int signum){
    switch(signum) {
        case SIGUSR1:
            flag_endDay = 1;
            break;
        case SIGTERM:
            flag_endSim = 1;
            break;
    }
}

void startDay(int qid, int semid) {    
    double p_serv = (double)rand() / RAND_MAX;
    if(flag_endDay){
        return;
    }
    if(flag_endSim){
        return;
    }
    if(p_serv < P_SERV_MIN || p_serv > P_SERV_MAX) {
        return;
    }
    
    block_signal();

    struct message_t msg_snd_to_erog, msg_rcv_from_erog;
    int n_requests = rand() % N_REQUESTS + 1;
    int list_serv[n_requests];

    crea_lista_serv(list_serv, n_requests);
    int ora = rand() % 10;

    long nanosec_per_hour = (long)N_NANO_SECS * ora * 60;
    struct timespec t_hour;
    t_hour.tv_sec = nanosec_per_hour / 1000000000;
    t_hour.tv_nsec = 0;

    nanosleep(&t_hour, NULL);

    for(int i = 0; i < n_requests && !check_signal() && (!flag_endDay && !flag_endSim); i++) {
        int num_serv = list_serv[i];
        if(datptr->serv_erog[num_serv-1] > 0) {
            msg_snd_to_erog.type_msg = NUM_SERV+1;
            msg_snd_to_erog.pid = getpid();
            
            snprintf(msg_snd_to_erog.msg, MSG_LENGTH, "%d", num_serv);
            send_msg(qid, &msg_snd_to_erog);
            
            unblock_signal();
            if(flag_endSim) return;

            if(receive_msg(qid, &msg_rcv_from_erog, getpid()) == -1) {
                reserve_sem(datptr->semid, 3);
                datptr->stat.n_serv_non_erog[num_serv-1] += 1;
                release_sem(datptr->semid, 3);

                break;
            }

            block_signal();

            int time = atoi(msg_rcv_from_erog.msg);
            clock_t start, end;
            double attesa;

            start = clock();

            reserve_sem(datptr->semid, 3);
            datptr->stat.t_erog_serv[num_serv-1] += time;
            datptr->stat.n_serv_erog[num_serv-1] += 1;
            datptr->utenti_in_attesa++;
            release_sem(datptr->semid, 3);

            unblock_signal();
            if(flag_endSim) return;

            if(reserve_sem(semid, num_serv-1) == -1){
                return;
            }

            struct message_t msg_snd_to_op, msg_rcv_from_op;

            msg_snd_to_op.type_msg = num_serv;
            msg_snd_to_op.pid = getpid();
            
            snprintf(msg_snd_to_op.msg, MSG_LENGTH, "%d", time);
            send_msg(qid, &msg_snd_to_op);

            if (receive_msg(qid, &msg_rcv_from_op, getpid()) == -1){
                break;
            }

            block_signal();

            end = clock();

            attesa = ((double)(end - start))/CLOCKS_PER_SEC;
            release_sem(semid, num_serv-1);

            reserve_sem(datptr->semid, 3);
            datptr->utenti_in_attesa--;
            datptr->stat.n_utenti_serviti[num_serv-1] += 1;
            datptr->stat.t_attesa_utenti[num_serv-1] += attesa;
            release_sem(datptr->semid, 3);

            unblock_signal();
        }
    }

    unblock_signal();
    
}

int main(int argc, char* argv[]) {
    /* Per distinguere gli utenti aggiunti dopo da quelli non.
       Gli utenti nuovi NON devono riservare/rilasciare nessun semaforo inizializzato dal direttore */
    int is_new_user = (argc > 3); 
    
    int semid = atoi(argv[2]);
    datptr = (Data*)attach_shm(atoi(argv[1]));
    int qid = datptr->qid;
    
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    sigaddset(&new_mask, SIGTERM);
    
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    srand(time(NULL) + getpid());

    if(!is_new_user)
        reserve_sem(datptr->semid, 0);                      //fine inizializzazione processo

    sem_operation(sops, datptr->semid, 2, 0, 0, 1);     //per iniziare giornata aspetta padre
    startDay(qid, semid);

    while(!flag_endSim){

        if(flag_endDay) {
            if(!is_new_user){
                reserve_sem(datptr->semid, 1); //segnale gestito
            }
            sem_operation(sops, datptr->semid, 2, 0, 0, 1); //inizio nuova giornata
            flag_endDay = 0;

            startDay(qid, semid);
            
        } else {
            sigsuspend(&old_mask);
        }
    }
    //reserve_sem(datptr->semid, 1);
    detach_shm(datptr);

    return 0;
}

void crea_lista_serv(int * list_serv, int n_requests){
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
}

int check_signal(){
    sigset_t pending_set;

    if(sigpending(&pending_set) != 0){
        perror("sigpending failed.");
        exit(EXIT_FAILURE);
    }

    return sigismember(&pending_set, SIGUSR1) || sigismember(&pending_set, SIGTERM);
}

void block_signal() {
    if(sigprocmask(SIG_BLOCK, &new_mask, &old_mask) < 0){
        perror("segnale non bloccato.");
        exit(EXIT_FAILURE);
    }
}

void unblock_signal() {
    if(sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0){
        perror("segnale non sbloccato in startDay.");
        exit(EXIT_FAILURE);
    }
}