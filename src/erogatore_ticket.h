#ifndef EROGA_TICKET_H
#define EROGA_TICKET_H

/**
 * @brief Stabilisce il tempo di erogazione di un servizio.
 *  
 * @param[in] num_serv  Numero del servizio a cui si fa riferimento.
 * @return              Restituisce il tempo di erogazione di un servizio.
 */

int eroga_ticket(int num_serv);

/**
 * @brief Gestisce l'arrivo di un segnale.
 *  
 * @param[in] signum    Segnale da gestire.
 */

void signal_handler(int signum);

#endif