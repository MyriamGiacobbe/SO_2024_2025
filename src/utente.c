#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"
#include "ipc/message_queue.h"
#include "common.h"

#define N_REQUESTS 3

Data* datptr;
struct sembuf sops;
int flag_handler = 0;
sigset_t new_mask, old_mask; 

typedef struct {
    int serv;
} Messaggio;

void endDay_handler(int signum){
    flag_handler = 1;
}

void startDay() {
    // scelta di andare

    double p_serv = (double)rand() / RAND_MAX;
    
    if(p_serv < P_SERV_MIN || p_serv > P_SERV_MAX)
        return;

    // scelta numero richieste di servizio
    
    int n_requests = rand() % N_REQUESTS + 1;
    printf("n requests: %d\n", n_requests);

    int list_serv[n_requests];
    for(int i = 0; i < n_requests;) {
        int random = rand() % NUM_SERV + 1;

        int occorrenza = 0;
        for(int j = 0; j < i && !occorrenza; j++) {
            if(list_serv[j] == random)
                occorrenza = 1;
        }

        if(!occorrenza) {
            list_serv[i] = random;
            i++;
        }   
    }

    // scelta orario
}

int main(int argc, char* argv[]) {
    int key;
    if((key = ftok(".", 'U')) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    int qid = create_queue(key);

    int sem_erog = create_sem(key, 1);

    init_sem(sem_erog, 0, 1);

    int sem_stat = create_sem(key, 1);

    init_sem(sem_stat, 0, 1);
    
    //datptr = (Data*)attach_shm(atoi(argv[1]));
    
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = endDay_handler;
    
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    
    sigaction(SIGUSR1, &sa, NULL);
    
    srand(time(NULL) + getpid());

    //reserve_sem(datptr->risorse.semid, 0);                      //fine inizializzazione processo
    //sem_operation(sops, datptr->risorse.semid, 2, 0, 0, 1);     //per iniziare giornata aspetta padre

    // decido se andare
    startDay();

    delete_queue(qid);
    //detach_shm(datptr);

    printf("[UTENTE] Ora termino\n");

    return 0;
}