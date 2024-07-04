#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

int requestsNumber = 0;
int workType = 4;

void printNumber(){
    for(int i = 0; i <= 100; i++){
        printf("%d\n", i);
    }
}

void pinntRequestsNumber(){
    printf("Number of requests: %d\n", requestsNumber);
}

void endProgram(){
    printf("terrorists win, program caput!\n");
    exit(0);
}

void actionFunction(int sig, siginfo_t *info, void *ucontext){
    requestsNumber++;
    workType = info->si_status;
    kill(info->si_pid, SIGUSR1);
}


int main(int argc, char* argv[]){
    pid_t PID = getpid();
    printf("catcher PID: %d\n", PID);

    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_sigaction = &actionFunction;
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &action, NULL);

    while(1){
        switch(workType){
            case 1:
                printNumber();
                workType = 4;
                break;
            case 2:
                pinntRequestsNumber();
                workType = 4;
                break;
            case 3:
                endProgram();
                workType = 4;
                break;
            default:
                pause();
                break;
        }
    }


}