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

int check_signal(int flag){
    sigset_t pending_set;
    int is_pending;

    if(sigpending(&pending_set) != 0){
        perror("sigpending failed.");
        exit(EXIT_FAILURE);
    }
    
    is_pending = sigismember(&pending_set, SIGUSR1);

    return is_pending;
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
    
    if(reserve_sem(semid_seats, serv-1) == -1){
        return;
    }

    n_attivi_g += 1;
    n_attivi_s += 1;
    
    if(sigprocmask(SIG_BLOCK, &new_mask, &old_mask) < 0){
        perror("segnale non bloccato.");
        exit(EXIT_FAILURE);
    }

    int flag = 0;
    struct message_t msg_snd, msg_rcv;

    while(!check_signal(1)){
        
        if(receive_msg(qid, &msg_rcv, serv) == -1)
            break;
        printf("[op]: sto servendo utente\n");

        
        long nanosec_per_min = N_NANO_SECS * atol(msg_rcv.msg);
        struct timespec t_hour;
        t_hour.tv_sec = nanosec_per_min / 1000000000;
        t_hour.tv_nsec = 0;
        
        nanosleep(&t_hour, NULL);
        printf("[op]: finito di servire, lo dico\n");
        msg_snd.type_msg = msg_rcv.pid;
        msg_snd.pid = getpid();
        
        snprintf(msg_snd.msg, MSG_LENGTH, "FATTO");
        send_msg(qid, &msg_snd);
        printf("[op]: L'ho detto\n");
        
        if(!flag){
            flag = goPause(serv-1, semid_seats);
            if(flag){
                printf("[op]: vado in pausa\n");
                n_pause_g += 1;
                n_pause_s += 1;
                break;
            }
        }
    }

    release_sem(semid_seats, serv-1);

    if(sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0){
        perror("segnale non sbloccato in startDay.");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    int qid = create_queue(KEY_MSG);

    int sem_stat = create_sem(KEY, 1);

    init_sem(sem_stat, 0, 1);

    srand(time(NULL) + getpid());
    int serv = rand() % NUM_SERV + 1;

    datptr = (Data*)attach_shm(atoi(argv[1]));

    struct sigaction sa_u1;
    struct sigaction sa_u2;
    bzero(&sa_u1, sizeof(sa_u1));
    sa_u1.sa_handler = endDay_handler;

    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    
    sigaction(SIGUSR1, &sa_u1, NULL);

    reserve_sem(datptr->semid, 0);                      //fine inizializzazione processo

    printf("[operatore %d] Sono inizializzato\n", getpid());

    sem_operation(sops, datptr->semid, 2, 0, 0, 1);     //per iniziare giornata aspetta padre

    printf("[operatore %d] Inizio la prima giornata\n", getpid());
    
    startDay(serv, atoi(argv[2]), qid);

    while(1){
        if(flag_handler) {

            reserve_sem(sem_stat, 0);
            datptr->stat.n_op_attivi_giorno += n_attivi_g;
            datptr->stat.n_op_attivi_sim += n_attivi_s;
            datptr->stat.n_pause_giorno += n_pause_g;
            datptr->stat.n_pause_sim += n_pause_s;
            release_sem(sem_stat, 0);

            reserve_sem(datptr->semid, 1); //segnale gestito 

            sem_operation(sops, datptr->semid, 2, 0, 0, 1); //inizio nuova giornata

            n_attivi_g = 0;
            n_pause_g = 0;

            startDay(serv, atoi(argv[2]), qid);

            release_sem(datptr->semid, 1); //ripristino del semaforo di gestione handler

            flag_handler = 0;
        }
    }

    detach_shm(datptr);

    return 0;
}