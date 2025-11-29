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

#define TOTAL_CHILD NOF_WORKERS + NOF_USERS + 1
#define WORKERS_USERS NOF_WORKERS + NOF_USERS
#define NUM_SERV 6

#define KEY_SEM  2024
#define KEY_SHM  2025

const char *fifo_name = "my_fifo";

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
    int operatore_sportello_giorno[NOF_WORKERS_SEATS];    
} Statistiche;

typedef struct {
    int semid;
    int qid;
    int serv_erog[NUM_SERV];
    int operatore_sportello[NOF_WORKERS_SEATS][NUM_SERV];
    int utenti_in_attesa;
	Statistiche stat;
} Data;


#endif