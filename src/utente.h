#ifndef UTENTE_H
#define UTENTE_H

void signal_handler(int signum);
int check_signal();
void block_signal();
void unblock_signal();
void startDay(int qid, int semid);
void crea_lista_serv(int * list_serv, int n_requests);

#endif