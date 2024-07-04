#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int global = 0;

int main(int argc, char *argv[]) {
    
    int local = 0;

    pid_t PID = fork();

    if(PID == -1)
    {
        return 1;
    }

    if(PID == 0){
        printf("child process\n");
        global++;
        local++;
        printf("child PID = %d, parent PID = %d\n", getpid(), getppid());
        printf("child's local = %d, child's global = %d\n", local, global);
        return execl("/bin/ls", "ls", "-l", argv[1], NULL);
    }
    else{
        int childStatus;
        wait(&childStatus);
        printf("parent process\n");
        printf("parent PID = %d, child PID = %d\n", getpid(), PID);
        printf("Child exit code: %d\n", WEXITSTATUS(childStatus));
        printf("parent's local = %d, parent's global = %d\n", local, global);
        if(WEXITSTATUS(childStatus) != 0){
            return 1;
        }
    }


    return 0;
}