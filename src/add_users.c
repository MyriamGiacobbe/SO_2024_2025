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


//UNICO PROBLEMA: FARLO PARTIRE NEL MODO GIUSTO DIRETTORE









/*
key_t generate_new_key(char c) {
    key_t key;
    if((key = ftok(".", c)) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    return key;
}
*/
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

    int semid_seats = create_sem(KEY_SEM, NUM_SERV);
    Data *shared_data;
    int shmid = create_shm(KEY_SHM, sizeof(Data));
    shared_data = (Data*)attach_shm(shmid);

    // --- INIZIO DEBUG ---
    printf("\n=== DEBUG ADD_USERS ===\n");
    printf("SHM ID ottenuto: %d\n", shmid);
    printf("Indirizzo memoria: %p\n", (void*)shared_data);
    printf("Valore letto semid (Barriere): %d\n", shared_data->semid);
    printf("Valore letto semid_seats (Operatori): %d\n", semid_seats);
    
    if (shared_data->semid == 0) {
        printf("!!! ERRORE CRITICO: Sto leggendo semid=0.\n");
        printf("!!! CAUSA 1: Mi sono collegato a una SHM nuova/vuota (problema chiavi).\n");
        printf("!!! CAUSA 2: Il direttore non ha ancora scritto in memoria.\n");
        exit(1); // Esco subito per non generare errori a catena
    }
    printf("=== FINE DEBUG ===\n\n");
    // --- FINE DEBUG ---

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
            execvp("./utente", args);
            perror("execvp utente");
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
    while(wait(NULL) > 0);
    detach_shm(shared_data);

    return 0;
}