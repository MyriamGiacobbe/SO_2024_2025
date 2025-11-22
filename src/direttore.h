#ifndef DIRETTORE_H
#define DIRETTORE_H
#define TOTAL_CHILD NOF_WORKERS + NOF_USERS + 1
#define WORKERS_USERS NOF_WORKERS + NOF_USERS

#include <stdio.h>
#include <sys/types.h>

void leggo_stat(FILE * file);
void init_sportelli(int semid);
void create_process(char* file_name);
key_t generate_new_key(char c);

#endif