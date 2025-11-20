#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"
#include "ipc/message_queue.h"
#include <stdlib.h>
#include <time.h>
#include "common.h"

#define KEY 2025

int numPause = NOF_PAUSE;

int n_attivi_g = 0, n_attivi_s = 0, n_pause_g = 0, n_pause_s = 0;
volatile sig_atomic_t flag_handler = 0;

Data* datptr;
struct sembuf sops;
sigset_t new_mask, old_mask; 

void endDay_handler(int signum){
    flag_handler = 1;
}

int check_signal(){
    sigset_t pending_set;

    if(sigpending(&pending_set) != 0){
        perror("sigpending failed.");
        exit(EXIT_FAILURE);
    }

    return sigismember(&pending_set, SIGUSR1);
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

int goPause(int semnum, int semid_seats) {
    double random = (double)rand() / RAND_MAX;
    
    if(numPause > 0 && random < 0.25){
        numPause--;
        return 1;
    }
    return 0;
}

void startDay(int serv, int semid_seats, int qid) {

    if(flag_handler) return;
    
    if(reserve_sem(semid_seats, serv-1) == -1)
        return;

    block_signal();

    n_attivi_g ++;
    n_attivi_s ++;


    struct message_t msg_snd, msg_rcv;

    while(!check_signal() && !flag_handler){

        unblock_signal();

        if(receive_msg(qid, &msg_rcv, serv) == -1)
            break;

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

        if(goPause(serv-1, semid_seats)){
            n_pause_g ++;
            n_pause_s ++;

            release_sem(semid_seats, serv-1);

            unblock_signal();
            return;
        }
    }

    unblock_signal();

    release_sem(semid_seats, serv-1);

}

int main(int argc, char* argv[]) {
    setbuf(stdout, NULL);

    int qid = create_queue(KEY_MSG);

    srand(time(NULL) + getpid());
    int serv = rand() % NUM_SERV + 1;

    datptr = (Data*)attach_shm(atoi(argv[1]));

    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = endDay_handler;
    sa.sa_flags = 0;                                    //SystemCall interrotte

    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    
    sigaction(SIGUSR1, &sa, NULL);

    reserve_sem(datptr->semid, 0);                      //fine inizializzazione processo

    sem_operation(sops, datptr->semid, 2, 0, 0, 1);     //per iniziare giornata aspetta padre
    
    startDay(serv, atoi(argv[2]), qid);

    while(1){
        if(flag_handler) {
            reserve_sem(datptr->semid, 3);
            datptr->stat.n_op_attivi_giorno += n_attivi_g;
            datptr->stat.n_op_attivi_sim += n_attivi_s;
            datptr->stat.n_pause_giorno += n_pause_g;
            datptr->stat.n_pause_sim += n_pause_s;
            release_sem(datptr->semid, 3);

            reserve_sem(datptr->semid, 1); //segnale gestito 

            sem_operation(sops, datptr->semid, 2, 0, 0, 1); //inizio nuova giornata

            n_attivi_g = 0;
            n_pause_g = 0;

            flag_handler = 0;

            startDay(serv, atoi(argv[2]), qid);
        } else {
            pause();
        }
    }

    detach_shm(datptr);

    return 0;
}