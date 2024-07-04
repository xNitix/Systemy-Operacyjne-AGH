#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int descendantsNumber = atoi(argv[1]);

    for (int i = 0; i < descendantsNumber; i++) {
        pid_t PID = fork();

        if (PID == 0){
            printf("PPID: %d, PID: %d\n", getppid(), getpid());
            exit(0);
        }
    }

    while(wait(NULL) >= 0){

    }

    printf("Descendants number: %d\n", descendantsNumber);
    return 0;
}