#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

void handlerFunction(int sig){
    printf("Otrzymano sygnał SIGUSR1!\n");
}

int main(int argc, char *argv[]){
    sigset_t mask;
    sigemptyset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    if(strcmp(argv[1], "none") == 0){
        // SIG_DEF ??
        signal(SIGUSR1, SIG_DFL);
    }
    else if(strcmp(argv[1], "ignore") == 0){
        signal(SIGUSR1, SIG_IGN);
    }
    else if(strcmp(argv[1], "handler") == 0){
        signal(SIGUSR1, &handlerFunction);
    }
    else if(strcmp(argv[1], "mask") == 0){
        sigaddset(&mask, SIGUSR1);
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }
    // kill(getpid(), SIGUSR1);
    raise(SIGUSR1);

    if(sigismember(&mask, SIGUSR1) == 1){
        printf("SIGUSR1 jest blokowany!\n");
    } else {
        printf("SIGUSR1 nie jest blokowany!\n");
    }

}