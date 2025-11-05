#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"
#include "ipc/message_queue.h"
#include "common.h"

/*
int flag_handler = 0;

void endDay_handler(int signum){
    flag_handler = 1;
}
    */

int main(int argc, char* argv[]) {
    Data* datptr;
    datptr = (Data*)attach_shm(atoi(argv[1]));

    
    //printf("[UTENTE] Ciao sono il figlio %d\n", getpid());
    reserve_sem(datptr->risorse.semid, 0);

    struct sembuf sops;
    sem_operation(sops, datptr->risorse.semid, 0, 0, 0, 1);

    //printf("%d\n", datptr->risorse.qid);
    struct sigaction sa; 
    bzero(&sa, sizeof(sa));
   // sa.sa_handler = endDay_handler;

   // sigaction(SIGUSR1, &sa, NULL);

    detach_shm(datptr);

    printf("[UTENTE] Ora termino\n");

    return 0;
}