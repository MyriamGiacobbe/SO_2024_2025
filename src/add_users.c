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

    int num_users = atoi(argv[1]);

    if(num_users <= 0) {
        fprintf(stderr, "No users\n");
        exit(EXIT_FAILURE);
    }

    int semid = create_sem(generate_new_key('D'), NUM_SERV);

    Data *shared_data;

    int shmid = create_shm(generate_new_key('I'), sizeof(shared_data));
    shared_data = (Data*)attach_shm(shmid);

    char shmid_str[32];
    snprintf(shmid_str, 32, "%d", shmid);
    char sem_str[32];
    snprintf(sem_str, 32, "%d", semid);

    char* args[] = {"utente", shmid_str, semid_str, NULL};

    pid_t pid;

    for(int i = 0; i < atoi(argv[1]); i++) {
        pid = fork();
        if(pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if(pid == 0) {
            execvp("../bin/utente", args);
            perror("execvp operatore");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}