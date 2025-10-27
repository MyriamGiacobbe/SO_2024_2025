#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"
#include "ipc/message_queue.h"
#include <stdlib.h>
#include <time.h>
#include "common.h"



int main(int argc, char* argv[]) {
    //printf("[OPERATORE] Ciao sono il figlio %d\n", getpid());

    srand(time(NULL) + getpid());
    int serv = rand() % NUM_SERV + 1;
    printf("Num random: %d\n", serv);

    reserve_sem(atoi(argv[1]), 0);

    
    struct sembuf sops;
    sem_operation(sops, atoi(argv[1]), 0, 0, 0, 1);
    
    Data* datptr;
    datptr = (Data*)attach_shm(atoi(argv[2]));

    for(int i = 0; i < NOF_WORKERS_SEATS; i++){
        if(datptr->sportelli[i] == serv){
            printf("Non sto lavorando ed erogo il servizio %d\n", datptr->sportelli[i]);
            reserve_sem(datptr->risorse.semid, i);
            printf("Sto lavorando allo sportello %d con servizio %d\n", i, datptr->sportelli[i]);
        }
    }

    //printf("%d\n", datptr->risorse.qid);

    detach_shm(datptr);

    printf("[OPERATORE] Ora termino\n");

    return 0;
}