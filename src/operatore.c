#include "operatore.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"
#include "ipc/message_queue.h"
#include <stdlib.h>
#include <time.h>
#include "common.h"


int numPause = NOF_PAUSE;
int n_attivi = 0, n_pause = 0;
int flag_endDay = 0, flag_endSim = 0;

Data* datptr;
struct sembuf sops;
sigset_t new_mask, old_mask; 

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

int goPause() {
    double random = (double)rand() / RAND_MAX;
    
    if(numPause > 0 && random < 0.25){
        numPause--;
        return 1;
    }
    return 0;
}

void startDay(int serv, int semid_seats, int qid) {
    if(flag_endDay){
        return;
    }
    if(flag_endSim){
        return;
    }
    if(reserve_sem(semid_seats, serv-1) == -1){
        return;
    }

    block_signal();
    reserve_sem(datptr->semid, 3);
    
    int flag_sportello = 0;
    int n_sportello = 0;

    for(; n_sportello < NOF_WORKERS_SEATS && !flag_sportello; n_sportello++) {
        if(datptr->operatore_sportello[n_sportello][serv-1] == 1) {
            flag_sportello = 1;
            datptr->operatore_sportello[n_sportello][serv-1] = 0;
            datptr->stat.operatore_sportello_giorno[n_sportello] += 1;
        }
    }
    n_sportello--;

    release_sem(datptr->semid, 3);
    n_attivi ++;
    struct message_t msg_snd, msg_rcv;

    while(!check_signal() && (!flag_endDay && !flag_endSim)){
        unblock_signal();
    
        if(flag_endSim) return;

        if(receive_msg(qid, &msg_rcv, serv) == -1){
            break;
        }

        block_signal();
        
        long nanosec_per_min = N_NANO_SECS * atol(msg_rcv.msg);
        struct timespec t_hour;
        t_hour.tv_sec = nanosec_per_min / 1000000000;
        t_hour.tv_nsec = 0;
        
        nanosleep(&t_hour, NULL);
        
        msg_snd.type_msg = msg_rcv.pid;
        msg_snd.pid = getpid();
        
        snprintf(msg_snd.msg, MSG_LENGTH, "FATTO");
        send_msg(qid, &msg_snd);

        if(goPause()){
            n_pause++;
            reserve_sem(datptr->semid, 3);
            datptr->operatore_sportello[n_sportello][serv-1] = 1;
            release_sem(datptr->semid, 3);

            release_sem(semid_seats, serv-1);
            unblock_signal();
            return;
        }
    }
    unblock_signal();
    release_sem(semid_seats, serv-1);
}

int main(int argc, char* argv[]) {

    srand(time(NULL) + getpid());
    int serv = rand() % NUM_SERV + 1;
    datptr = (Data*)attach_shm(atoi(argv[1]));
    int qid = datptr->qid;

    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;                                    //SystemCall interrotte

    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    sigaddset(&new_mask, SIGTERM);
    
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    reserve_sem(datptr->semid, 0);                      //fine inizializzazione processo
    sem_operation(sops, datptr->semid, 2, 0, 0, 1);     //per iniziare giornata aspetta padre
    startDay(serv, atoi(argv[2]), qid);

    while(!flag_endSim){

        if(flag_endDay) {
            reserve_sem(datptr->semid, 3);
            scrivo_stat();
            release_sem(datptr->semid, 3);
            reserve_sem(datptr->semid, 1); //segnale gestito

            sem_operation(sops, datptr->semid, 2, 0, 0, 1); //inizio nuova giornata

            n_attivi = 0;
            n_pause = 0;
            flag_endDay = 0;

            startDay(serv, atoi(argv[2]), qid);
            
        } else {
            sigsuspend(&old_mask);
        }
    }
    reserve_sem(datptr->semid, 3);
    scrivo_stat();
    release_sem(datptr->semid, 3);
    //reserve_sem(datptr->semid, 1);
    detach_shm(datptr);

    return 0;
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

void scrivo_stat() {
    datptr->stat.n_op_attivi_giorno += n_attivi;
    datptr->stat.n_op_attivi_sim += n_attivi;
    datptr->stat.n_pause_sim += n_pause;
}