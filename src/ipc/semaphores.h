#ifndef SEM_H
#define SEM_H

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
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
};

/**
 * @brief Crea un semaforo
 *  
 * @param[in] k		  Chiave associata al semaforo.
 * @param[in] nsems	  Numero di semafori da creare nel set.
 * @return			  Restituisce l'ID del semaforo.
 */

int create_sem(key_t k, int nsems);

/**
 * @brief Inizializza un semaforo
 *  
 * @param[in] semid	  ID del semaforo.
 * @param[in] semnum  Numero del semaforo nel set.
 */

void init_sem(int semid, int semnum, int val);

/**
 * @brief Svolge un'operazione su un semaforo
 *  
 * @param[in] sops	  Struttura che contiene le operazioni eseguibili dal semaforo.
 * @param[in] semid	  ID del semaforo.
 * @param[in] semnum  Numero del semaforo nel set.
 * @param[in] semflg  Flag dell'operazione.
 * @param[in] op  	  Operazione del semaforo.
 * @param[in] nsem	  Numero del semaforo su cui svolgere l'operazione.
 */

void sem_operation(struct sembuf sops, int semid, int semnum, int semflg, int op, int nsem);

/**
 * @brief Inizializza un semaforo
 *  
 * @param[in] semid	  ID del semaforo.
 * @param[in] semnum  Numero del semaforo nel set.
 */

int reserve_sem(int semid, int semnum);

/**
 * @brief Inizializza un semaforo
 *  
 * @param[in] semid	  ID del semaforo.
 * @param[in] semnum  Numero del semaforo nel set.
 */

void release_sem(int semid, int semnum);

/**
 * @brief Inizializza un semaforo
 *  
 * @param[in] semid	  ID del semaforo.
 */

void delete_sem(int semid);

#endif