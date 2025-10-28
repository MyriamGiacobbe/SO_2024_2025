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

void goPause(int semnum) {

    srand(time(NULL));
    double random = (double)rand() / RAND_MAX;

    if(random > 0.25)
        return;
    
    if(numPause > 0){
        numPause--;
        printf("PAUSA\n");
        release_sem(datptr->risorse.semid, semnum);
    }
}

void startDay(int serv) {
    reserve_sem(datptr->risorse.semid, serv-1);

    goPause(serv-1);
}

int main(int argc, char* argv[]) {
    srand(time(NULL) + getpid());
    int serv = rand() % NUM_SERV + 1;

    datptr = (Data*)attach_shm(atoi(argv[2]));

    reserve_sem(atoi(argv[1]), 0);

    sem_operation(sops, atoi(argv[1]), 0, 0, 0, 1);     //waitforzero -> aspetta l'inizializzazione di tutti i fratelli

    startDay(serv);    

    detach_shm(datptr);

    return 0;
}