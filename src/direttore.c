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

void init() {
    for(int i = 1, i <= NOF_USERS, i++) {
        pid_t pid = fork();
    }
    
}

int main(int argc, char *argv[]) {
/*    
    FILE *fp = fopen(argv[1], "r");

    if(fp == NULL) {
        perror("Errore apertura config");
        exit(EXIT_FAILURE);
    }

    char line[64];

    while(fgets(line, sizeof(line), fp)) {
        
    }
*/

    

    init();

}