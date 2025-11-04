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
Data* datptr;
struct sembuf sops;
int flag_handler = 0;

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

int goPause(int semnum) {

    srand(time(NULL));
    double random = (double)rand() / RAND_MAX;

    if(random > 0.75)
        return 0;
    
    if(numPause > 0){
        numPause--;
        printf("PAUSA\n");
        release_sem(datptr->risorse.semid, semnum);
        return 1;
    }
}

void startDay(int serv, int semid_seats) {
    reserve_sem(semid_seats, serv-1);

    while(!check_signal()){
        if(goPause(serv-1)){
            break;
        }
        else{
            sleep(1);
        }
    }
    release_sem(semid_seats, serv-1);
    printf("Scrivo statistiche\n");
}

int main(int argc, char* argv[]) {
    srand(time(NULL) + getpid());
    int serv = rand() % NUM_SERV + 1;

    datptr = (Data*)attach_shm(atoi(argv[1]));

    struct sigaction sa;
    sigset_t new_mask, old_mask; 
    bzero(&sa, sizeof(sa));
    sa.sa_handler = endDay_handler;

    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);

    if(sigprocmask(SIG_BLOCK, &new_mask, &old_mask) < 0){
        perror("segnale non bloccato.");
        exit(EXIT_FAILURE);
    }
    
    sigaction(SIGUSR1, &sa, NULL);

    reserve_sem(datptr->risorse.semid, 0);
    sem_operation(sops, datptr->risorse.semid, 0, 0, 0, 3);     //waitforzero -> aspetta l'inizializzazione di tutti i fratelli
    
    startDay(serv, atoi(argv[2]));    

    if(sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0){
        perror("segnale non bloccato.");
        exit(EXIT_FAILURE);
    }
/*
    while(flag_handler()){
        printf("Sto per iniziare una bellissima giornata\n");
        reserve_sem(datptr->risorse.semid, 1); //segnale in pending 
        sem_operation(sops, datptr->risorse.semid, 2, 0, 0, 1);
        printf("Hanno tutti gestito il segnale\n");
        sem_operation(sops, datptr->risorse.semid, 2, 0, 1, 1);
        startDay(serv);
    }
*/    

    detach_shm(datptr);

    return 0;
}