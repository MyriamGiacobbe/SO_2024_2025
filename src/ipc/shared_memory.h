#ifndef SHM_H
#define SHM_H

int create_shm(key_t k, size_t size);
void* attach_shm(int shmid);
void detach_shm(void* mem);
void remove_shm(int shmid);

#endif