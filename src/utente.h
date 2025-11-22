#ifndef UTENTE_H
#define UTENTE_H

/**
 * @brief Gestisce l'arrivo di un segnale.
 *  
 * @param[in] signum    Segnale da gestire.
 */
void signal_handler(int signum);

/**
 * @brief  Controlla se ci sono segnali in attesa di essere gestiti.
 *
 * @return Restituisce 1 se ci sono segnali in attesa di essere gestiti, altrimenti 0.
 */
int check_signal();

/**
 * @brief Blocca la gestione dei segnali.
 */

void block_signal();

/**
 * @brief Sblocca la gestione dei segnali.
 */
void unblock_signal();

/**
 * @brief  Inizia una nuova giornata lavorativa.
 *
 * @param  qid   ID della coda di messaggi.
 * @param  semid ID del semaforo.
 *
 */

void startDay(int qid, int semid);

/**
 * @brief  Aggiunge alla lista i servizi che gli utenti decidono di richiedere.
 *
 * @param  list_serv   Puntatore alla lista.
 * @param  n_request   Dimensione della lista.
 *
 */
void crea_lista_serv(int * list_serv, int n_requests);

#endif