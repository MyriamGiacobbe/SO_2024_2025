#ifndef SHM_H
#define SHM_H

#define ERROR fprintf(stderr, \
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int create_shm(key_t k, size_t size);
void* attach_shm(int shmid);
void detach_shm(void* mem);
void remove_shm(int shmid);

#endif