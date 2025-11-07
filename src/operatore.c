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
    // srand(time(NULL));
    double random = (double)rand() / RAND_MAX;

    if(random > 0.15)
        return 0;
    
    if(numPause > 0){
        numPause--;
        printf("PAUSA\n");
        release_sem(semid_seats, semnum);
        return 1;
    }
    return 0;
}

void startDay(int serv, int semid_seats) {
    printf("[DEBUG - OPERATORE] Inizio una nuova giornata\n");
    
    /*if(semctl(semid_seats, serv-1, GETVAL) >= NUM_SERV + 1){
        if(sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0){
            perror("segnale non sbloccato in startDay.");
            exit(EXIT_FAILURE);
        }
        return;
    }*/
    if(reserve_sem(semid_seats, serv-1) == -1){
        if(errno == EINTR){
            return;
        }else{
            ERROR
        }
    }
    
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
    if(sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0){
        perror("segnale non sbloccato in startDay.");
        exit(EXIT_FAILURE);
    }

    // Ora che il loop è finito, sblocchiamo il segnale
    
    // APPENA QUESTA RIGA FINISCE, l'handler viene eseguito
    // e flag_handler viene impostato a 1 (se il segnale era pendente)
    if(!flag)
        release_sem(semid_seats, serv-1);
    
    // Ora puoi controllare il flag qui, se vuoi
    
    
    //printf("Scrivo statistiche\n");
}
int main(int argc, char* argv[]) {
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

    //printf("Qui tutto ok\n");

    while(1){
        if(flag_handler) {
            //printf("Handler eseguito dentro startDay!\n");
            reserve_sem(datptr->risorse.semid, 1); //segnale in pending 

            printf("[DEBUG - OPERATORE] Segnale di fine giornata gestito\n");

            sem_operation(sops, datptr->risorse.semid, 2, 0, 0, 1);

            printf("[DEBUG - OPERATORE] Posso iniziare\n");

            startDay(serv, atoi(argv[2]));

            release_sem(datptr->risorse.semid, 1);

            flag_handler = 0;
        }
    }

    detach_shm(datptr);

    return 0;
}