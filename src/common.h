#ifndef COMMON_H
#define COMMON_H

#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef EXPLODE
#include "config_explode.h"
#endif

#ifdef TIMEOUT
#include "config_timeout.h"
#endif

#define ERROR fprintf(stderr, \
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));

#define NUM_SERV 6

#define KEY_SHM 2025
#define KEY_SEM 2026
#define KEY_MSG 2027

typedef struct {
    int n_utenti_serviti[NUM_SERV];
    int n_serv_erog[NUM_SERV];
    int n_serv_non_erog[NUM_SERV];
    double t_attesa_utenti[NUM_SERV];
    double t_erog_serv[NUM_SERV];
    int n_op_attivi_giorno;
    int n_op_attivi_sim;
    int n_pause_giorno;
    int n_pause_sim;
    double operatore_sportello_giorno;
} Statistiche;

typedef struct {
    int arr[NOF_WORKERS_SEATS];
	Statistiche stat;
} Data;

typedef struct{
    int shmid;
    int qid;
    int semid;
} Risorse;

#endif