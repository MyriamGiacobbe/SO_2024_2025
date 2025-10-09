#ifndef SEM_H
#define SEM_H

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
};

int create_sem(key_t k, int senum);
int reserve_sem(int semid, int semnum);
int release_sem(int semid, int semnum);
void deleate_sem(int semid);

#endif