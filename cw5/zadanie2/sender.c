#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    // atoi - string to int
    pid_t catcherPID = atoi(argv[1]);
    int catcherState = atoi(argv[2]);

    union sigval sigVal = {
    catcherState
    };
    
    sigqueue(catcherPID, SIGUSR1, sigVal);

    sigset_t mask;
    sigemptyset(&mask);

    sigsuspend(&mask);
    return 0;
}