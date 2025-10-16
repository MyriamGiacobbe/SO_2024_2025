#include "shared_memory.h"
#include "../common.h"


int create_shm(key_t k, size_t size) {
    int shmid = shmget(k, size, 0600);
    if(shmid == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }
    return shmid;
}

void* attach_shm(int shmid) {
    void* shared_data = (void*)shmat(shmid, NULL, 0);
    if(errno) {
        ERROR
        exit(EXIT_FAILURE);
    }
}

void detach_shm(void* shared_data) {
    if(shmdt(shared_data) == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }
}

void remove_shm(int shmid) {
    if(shmctl(shmid, IPC_RMID, NULL) == -1) {
        ERROR
        exit(EXIT_FAILURE);
    }
}