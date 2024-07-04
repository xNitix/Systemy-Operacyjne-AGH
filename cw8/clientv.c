#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <sys/wait.h>
#include "buffer.h"

#define N 5
#define SHM_KEY 1234
#define SEM_KEY 5678

void sem_op(int semid, int sem_num, int op) {
    struct sembuf sem_op;
    sem_op.sem_num = sem_num;
    sem_op.sem_op = op;
    sem_op.sem_flg = 0;
    semop(semid, &sem_op, 1);
}

int main() {
    int shmid = shmget(SHM_KEY, sizeof(Buffer), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    Buffer *buffer = shmat(shmid, NULL, 0);
    if (buffer == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    int semid = semget(SEM_KEY, 3, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < N; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            srand(getpid() * time(NULL));
            while (1) {
                sem_op(semid, 0, -1); // empty--
                sem_op(semid, 2, -1); // used--

                char task[11];
                for (int i = 0; i < 10; ++i) {
                    task[i] = 'a' + rand() % 26;
                }
                task[10] = '\0';
                strcpy(buffer->tasks[buffer->in], task);
                buffer->in = (buffer->in + 1) % BUFFER_SIZE;

                sem_op(semid, 2, 1); // used++
                sem_op(semid, 1, 1); // full++

                printf("PID: %d zlecilem wydruk: %s\n", getpid(), task);
                sleep(rand() % 6 + 1);
            }
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < N; ++i) {
        wait(NULL);
    }

    return 0;
}
