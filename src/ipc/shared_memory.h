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
#include <unistd.h>

/**
*@brief Crea una memoria condivisa
*
*@param [in] k		Chiave associata alla memoria condivisa
*@param [in] size	Dimensione del segmento di memoria
*@return restituisce l'ID della memoria condivisa
*/
int create_shm(key_t k, size_t size);

/**
*@brief Attacca il segmento di memoria alla struttura
*
*@param [in] shmid	ID della memoria condivisa
*@return restituisce il puntatore all'aria di memoria
*/
void* attach_shm(int shmid);

/**
*@brief Stacca la struttura dal segmento di memoria
*
*@param [in] mem	Struttura da staccare
*/
void detach_shm(void* mem);

/**
*@brief Rimuove l'aria di memoria
*
*@param [in] shmid	ID della memoria condivisa
*/
void remove_shm(int shmid);

#endif