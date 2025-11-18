#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"
#include "ipc/message_queue.h"
#include "common.h"

Data* datptr;
struct sembuf sops;
sigset_t new_mask, old_mask;

void reap_child_handler(int signum) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int eroga_ticket(int num_serv) {
    switch(num_serv) {
        case 1:
            return (rand() % (15-5+1) + 5);
        case 2:
        case 4:
            return (rand() % (12-4+1) + 4);
        case 3:
            return (rand() % (9-3+1) + 3);
        case 5:
        case 6:
            return (rand() % (30-10+1) + 10);
        default:
            return -1;
    }
}

int main(int argc, char* argv[]) {

    struct message_t msg_rcv;

    int qid = create_queue(KEY_MSG);

    datptr = (Data*)attach_shm(atoi(argv[1]));
    
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &sa, NULL);

    struct sigaction sa_chld;
    bzero(&sa_chld, sizeof(sa_chld));
    sa_chld.sa_handler = reap_child_handler;
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);
    
    srand(time(NULL) + getpid());

    reserve_sem(datptr->risorse.semid, 0);                      //fine inizializzazione processo

    printf("[DEBUG - EROGATORE] reserve_sem\n");

    sem_operation(sops, datptr->risorse.semid, 2, 0, 0, 1);     //per iniziare giornata aspetta padre

    printf("[DEBUG - EROGATORE] sem_operation\n");

    while(1) {
        if(msgrcv(qid, &msg_rcv, MSG_LENGTH, 0, 0) < 0){
            if(errno == EINTR)
                continue;
            else {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
        }

        printf("[DEBUG - EROGATORE] receive_msg\n");

        pid_t pid = fork();
        if(pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if(pid == 0) {
            srand(time(NULL) + getpid());
            
            struct message_t msg_snd;
            msg_snd.type_msg = msg_rcv.pid;
            msg_snd.pid = getpid();

            int time;
            if ((time = eroga_ticket(atoi(msg_rcv.msg))) == -1) {
                fprintf(stderr, "eroga_ticket failed\n");
                exit(EXIT_FAILURE);
            }

            snprintf(msg_snd.msg, MSG_LENGTH, "%d", time);

            printf("[EROGATORE] %s\n", msg_snd.msg);

            send_msg(qid, &msg_snd);

            exit(EXIT_SUCCESS);
        }
    }

    return 0;
}