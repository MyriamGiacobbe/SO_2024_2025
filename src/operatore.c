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

void goPause(int semid, int semnum) {

    srand(time(NULL));
    double random = (double)rand() / RAND_MAX;

    if(random > 0.25)
        return;
    
    if(numPause > 0){
        numPause--;
        printf("PAUSA\n");
        release_sem(semid, semnum);
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL) + getpid());
    int serv = rand() % NUM_SERV + 1;

    reserve_sem(atoi(argv[1]), 0);

    struct sembuf sops;
    sem_operation(sops, atoi(argv[1]), 0, 0, 0, 1);
    
    Data* datptr;
    datptr = (Data*)attach_shm(atoi(argv[2]));

    int count = 0;
    int occ[NOF_WORKERS_SEATS];
    for(int i = 0; i < NOF_WORKERS_SEATS; i++) {
        if(datptr->sportelli[i] == serv){
            occ[count] = i;
            printf("occ[%d] = %d\n", count, i);
            count++;
        }
    }
    
    int i = 0;
    while(1) {
        if(i <= count) {
            if(semctl(datptr->risorse.semid, occ[i], GETVAL) == 1){
                reserve_sem(datptr->risorse.semid, occ[i]);
                break;
            }
            i++;
        } 
        i = 0;
    }

    goPause(datptr->risorse.semid, i);

    detach_shm(datptr);

    return 0;
}