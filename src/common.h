#ifndef COMMON_H
#define COMMON_H

#define ERROR fprintf(stderr, \
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));

#define N_SERV 6

#define KEY_SHM 2025
#define KEY_SEM 2026
#define KEY_msg 2027

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
	Statistiche stat;
} Data;

typedef struct{
    int shmid;
    int qid;
    int semid;
} Risorse;

#endif