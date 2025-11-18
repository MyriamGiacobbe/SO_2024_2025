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

#define NUM_SERV 6
#define UO_GROUP 

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

typedef struct{
    int qid;
    int semid;
} Risorse;

typedef struct {
    //pid_t pid[];
    int serv_erog[NUM_SERV];
	Statistiche stat;
    Risorse risorse;
} Data;


#endif