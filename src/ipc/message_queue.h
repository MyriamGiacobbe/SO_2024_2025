#ifndef Q_MSG_H
#define Q_MSG_H

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
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define KEY_MSG 2025

#define MSG_LENGTH 120

struct message_t{
    long type_msg;
    pid_t pid;
    char msg[MSG_LENGTH];
};

/**
*@brief Crea una coda di messaggi
*
*@param [in] k	Chiave associata alla coda di messaggi
*@return restituisce l'ID della coda di messaggi
*/
int create_queue(key_t k);

/**
*@brief Permette di inviare un messaggio
*
*@param [in] qid	ID della coda di messaggi
*@param [in] msg	Messaggio da inviare
*/
void send_msg(int qid, struct message_t *msg);

/**
*@brief Permette di ricevere un messaggio
*
*@param [in] qid	ID della coda di messaggi
*@param [in] msg	Struttura del messaggio dove salvare le informazioni ricevute
*@param [in] mtype	Tipo del messaggio
*@return restituisce -1 in caso di EINTR, altrimenti 0
*/
int receive_msg(int qid, struct message_t *msg, long mtype);

/**
*@brief Permette di ricevere un messaggio
*
*@param [in] qid	ID della coda di messaggi
*/
void delete_queue(int qid);

#endif