#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "common.h"
#include "ipc/message_queue.h"
#include "ipc/semaphores.h"
#include "ipc/shared_memory.h"

key_t generate_new_key(char c) {
    key_t key;
    if((key = ftok(".", c)) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    return key;
}

int main(int argc, char* argv[]) {

    if(argc < 2) {
        fprintf(stderr, "Too few argouments\n");
        exit(EXIT_FAILURE);
    }

    int fd;
    int num_users = atoi(argv[1]);
    if(num_users <= 0) {
        fprintf(stderr, "No users\n");
        exit(EXIT_FAILURE);
    }
    
    if(setpgid(0, 0) == -1) {
        perror("setpgid add_users");
    }
    pid_t pgid = getpid();

    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int semid_seats = create_sem(generate_new_key('D'), NUM_SERV);
    Data *shared_data;
    int shmid = create_shm(generate_new_key('I'), sizeof(shared_data));
    shared_data = (Data*)attach_shm(shmid);

    char shmid_str[32];
    snprintf(shmid_str, 32, "%d", shmid);
    char semid_seats_str[32];
    snprintf(semid_seats_str, 32, "%d", semid_seats);

    char* args[] = {"utente", shmid_str, semid_seats_str, "NEW", NULL};
    pid_t pid;
    for(int i = 0; i < num_users; i++) {
        pid = fork();
        if(pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if(pid == 0) {
            if(setpgid(0, pgid) < 0) {
                perror("setpgid add_users");
                exit(EXIT_FAILURE);
            }
            execvp("../bin/utente", args);
            perror("execvp operatore");
            exit(EXIT_FAILURE);
        }
    }

    int n = 0;
    char buff[32];
    while(1) {
        if((fd = open(fifo_name, O_RDONLY)) == -1) {
            perror("open");
            break;
        }
        n = read(fd, buff, 32);
        close(fd);

        if(n > 0) {
            if(strcmp(buff, "day") == 0)
                kill(-pgid, SIGUSR1);
            if(strcmp(buff, "sim") == 0) {
                kill(-pgid, SIGTERM);
                break;
            }
        } else {
            break;
        }
    }
    detach_shm(shared_data);
    while(wait(NULL) > 0);

    return 0;
}