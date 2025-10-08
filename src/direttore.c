#include <stdio.h>

#define SIM_DURATION = 5
#define N_NANO_SECS = 5
#define EXPLODE_THRESHOLD = 50
#define NOF_USERS = 15
#define NOF_WORKERS = 10
#define NOF_WORKERS_SEATS = 7
#define NOF_PAUSE = 10
#define P_SERV_MIN = 0.2
#define P_SERV_MAX = 0.9

#define KEY_SHM 2025
#define KEY_SEM 2026

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
} stat;

int main() {


}