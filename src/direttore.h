#ifndef DIRETTORE_H
#define DIRETTORE_H

#include <stdio.h>
#include <sys/types.h>

/**
*@brief Legge le statistiche dall'aria di memoria condivisa e le inserisce in un file csv
*
*@param [in] file   puntatore al file .csv
*/
void leggo_stat(FILE * file);

/**
*@brief Inizializza gli sportelli
*
*@param [in] semid  ID del set di semafori
*/
void init_sportelli(int semid);

/**
*@brief Crea processi figli
*
*@param [in] file_name  nome del file da eseguire
*/
void create_process(char* file_name);

/**
*@brief Genera una nuova chiave
*
*@param [in] c  carattere per la generazione
*@return restituisce la chiave generata
*/
key_t generate_new_key(char c);

#endif