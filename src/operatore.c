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

void startDay(int serv) {
    reserve_sem(datptr->risorse.semid, serv-1);

    while(1){
        if(goPause(serv-1)){
            break;
        }
        else{
            sleep(1);
        }
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL) + getpid());
    int serv = rand() % NUM_SERV + 1;

    datptr = (Data*)attach_shm(atoi(argv[2]));

    reserve_sem(atoi(argv[1]), 0);

    sem_operation(sops, atoi(argv[1]), 0, 0, 0, 1);     //waitforzero -> aspetta l'inizializzazione di tutti i fratelli

    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = endDay_handler;

    sigaction(SIGUSR1, &sa, NULL);

    startDay(serv);    

    while(1){
        if(flag_handler){
            flag_handler = 0;
            printf("Sto per iniziare una bellissima giornata\n");
            reserve_sem(atoi(argv[1]), 0);
            sem_operation(sops, atoi(argv[1]), 0, 0, 0, 1);
            printf("Hanno tutti gestito il segnale\n");
            sem_operation(sops, atoi(argv[1]), 0, 0, 1, 1);
            startDay(serv);
            
        }
    }

    detach_shm(datptr);

    return 0;
}