#ifndef OPERATORE_H
#define OPERATORE_H

void signal_handler(int signum);
int check_signal();
void block_signal();
void unblock_signal();
int goPause(int semnum, int semid_seats);
void startDay(int serv, int semid_seats, int qid);
void scrivo_stat();

#endif