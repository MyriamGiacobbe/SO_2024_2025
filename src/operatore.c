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

int n_attivi_g = 0;
int n_attivi_s = 0;
int n_pause_g = 0;
int n_pause_s = 0;

Data* datptr;
struct sembuf sops;
int flag_handler = 0;
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
    int is_pending = sigismember(&pending_set, SIGUSR1);
    return is_pending;
}

int goPause(int semnum, int semid_seats) {
    double random = (double)rand() / RAND_MAX;

    if(random > 0.25)
        return 0;
    
    if(numPause > 0){
        numPause--;
        //printf("PAUSA\n");
        release_sem(semid_seats, semnum);

        n_pause_g += 1;
        n_pause_s += 1;

        return 1;
    }
    return 0;
}

void startDay(int serv, int semid_seats) {
    
    if(reserve_sem(semid_seats, serv-1) == -1){
        if(errno == EINTR){
            return;
        } else{
            ERROR
        }
    }

    n_attivi_g += 1;
    n_attivi_s += 1;
    
    if(sigprocmask(SIG_BLOCK, &new_mask, &old_mask) < 0){
        perror("segnale non bloccato.");
        exit(EXIT_FAILURE);
    }

    int flag = 0;
    while(!check_signal()){
        sleep(5);
        if(!flag){
            flag = goPause(serv-1, semid_seats);
        }
    }

    if(!flag)
        release_sem(semid_seats, serv-1);

    if(sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0){
        perror("segnale non sbloccato in startDay.");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    int sem_stat = create_sem(KEY, 1);

    init_sem(sem_stat, 0, 1);

    srand(time(NULL) + getpid());
    int serv = rand() % NUM_SERV + 1;

    datptr = (Data*)attach_shm(atoi(argv[1]));

    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = endDay_handler;

    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    
    sigaction(SIGUSR1, &sa, NULL);

    reserve_sem(datptr->risorse.semid, 0);                      //fine inizializzazione processo
    sem_operation(sops, datptr->risorse.semid, 2, 0, 0, 1);     //per iniziare giornata aspetta padre
    
    startDay(serv, atoi(argv[2]));

    while(1){
        if(flag_handler) {

            reserve_sem(sem_stat, 0);
            datptr->stat.n_op_attivi_giorno += n_attivi_g;
            datptr->stat.n_op_attivi_sim += n_attivi_s;
            datptr->stat.n_pause_giorno += n_pause_g;
            datptr->stat.n_pause_sim += n_pause_s;
            release_sem(sem_stat, 0);

            reserve_sem(datptr->risorse.semid, 1); //segnale gestito 

            sem_operation(sops, datptr->risorse.semid, 2, 0, 0, 1); //inizio nuova giornata

            n_attivi_g = 0;
            n_pause_g = 0;

            startDay(serv, atoi(argv[2]));

            release_sem(datptr->risorse.semid, 1); //ripristino del semaforo di gestione handler

            flag_handler = 0;
        }
    }

    detach_shm(datptr);

    return 0;
}