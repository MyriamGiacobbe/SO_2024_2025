#ifndef OPERATORE_H
#define OPERATORE_H

/**
*@brief Gestisce l'arrivo di un segnale
*
*@param [in] signum segnale da gestire
*/
void signal_handler(int signum);

/**
*@brief Controlla se ci sono segnali in attesa di essere gestiti
*
*@return restituisce 1 se ci sono segnali in attesa di essere gestiti, altrimenti 0
*/
int check_signal();

/**
*@brief Blocca la gestione dei segnali
*/
void block_signal();

/**
*@brief Sblocca la gestione dei segnali
*/
void unblock_signal();

/**
*@brief Decide se un operatore va in pausa
*
*@return restituisce 1 se l'operatore va in in pausa, altrimenti 0
*/
int goPause(int semnum, int semid_seats);

/**
*@brief Inizia una giornata lavorativa
*
*@param [in] serv           numero del servizio che eroga l'operatore
*@param [in] semid_seats    ID del set di semafori che rappresenta gli sportelli
*@param [in] qid            ID della coda di messaggi
*/
void startDay(int serv, int semid_seats, int qid);

/**
*@brief Aggiorna le statistiche
*/
void scrivo_stat();

#endif