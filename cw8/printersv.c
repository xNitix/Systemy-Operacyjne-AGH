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

#define M 3
#define SHM_KEY 1234
#define SEM_KEY 5678

void sem_op(int semid, int sem_num, int op) {
    struct sembuf sem_op;
    sem_op.sem_num = sem_num;
    sem_op.sem_op = op;
    sem_op.sem_flg = 0;
    // semop(int semid, struct sembuf *sops, size_t nsops)
    // semid - identyfikator tablicy semaforow
    // sops - wskaznik do tablicy struktur sembuf
    // nsops - liczba elementow w tablicy sops
    // uzywamy tego by zmienic wartosc semafora na podana w sem_op
    semop(semid, &sem_op, 1);
}

int main() {
    // shmget(key_t key, size_t size, int shmflg)
    // key - klucz pamieci wspoldzielonej
    // size - rozmiar pamieci wspoldzielonej
    // shmflg - flagi tworzenia pamieci wspoldzielonej
    // zwraca identyfikator pamieci wspoldzielonej
    int shmid = shmget(SHM_KEY, sizeof(Buffer), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // shmat(int shmid, const void *shmaddr, int shmflg)
    // shmid - identyfikator pamieci wspoldzielonej
    // shmaddr - adres pamieci wspoldzielonej
    // shmflg - flagi dostepu do pamieci wspoldzielonej
    // zwraca adres pamieci wspoldzielonej
    // uzywamy tego by dostac dostep do pamieci wspoldzielonej
    Buffer *buffer = shmat(shmid, NULL, 0);
    if (buffer == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // semget(key_t key, int nsems, int semflg)
    // key - klucz tablicy semaforow
    // nsems - liczba semaforow w tablicy
    // semflg - flagi tworzenia tablicy semaforow
    // zwraca identyfikator tablicy semaforow
    int semid = semget(SEM_KEY, 3, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    // semctl(int semid, int semnum, int cmd, ...)
    // semid - identyfikator tablicy semaforow
    // semnum - numer semafora w tablicy
    // cmd - polecenie
    // uzywamy tego by ustawic wartosc semafora
    semctl(semid, 0, SETVAL, BUFFER_SIZE); // empty
    semctl(semid, 1, SETVAL, 0);           // full
    semctl(semid, 2, SETVAL, 1);           // used

    buffer->in = 0;
    buffer->out = 0;

    for (int i = 0; i < M; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            while (1) {
                // sem_op(int semid, int sem_num, int op)
                // semid - identyfikator tablicy semaforow
                // sem_num - numer semafora w tablicy
                // op - wartosc do zmiany
                // uzywamy tego by zmienic wartosc semafora
                sem_op(semid, 1, -1); // full--
                sem_op(semid, 2, -1); // used--

                char task[11];
                strcpy(task, buffer->tasks[buffer->out]);
                buffer->out = (buffer->out + 1) % BUFFER_SIZE;

                sem_op(semid, 2, 1); // used++
                sem_op(semid, 0, 1); // empty++

                for (int i = 0; i < 10; ++i) {
                    printf("Drukarka %d: znak: %c\n", getpid(), task[i]);
                    sleep(1);
                }
                printf("\n");
            }
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < M; ++i) {
        wait(NULL);
    }

    // semctl(int semid, int semnum, int cmd, ...)
    // semid - identyfikator tablicy semaforow
    // semnum - numer semafora w tablicy
    // cmd - polecenie
    // uzywamy tego by usunac tablice semaforow
    semctl(semid, 0, IPC_RMID, 0);

    // shmctl(int shmid, int cmd, struct shmid_ds *buf)
    // shmid - identyfikator pamieci wspoldzielonej
    // cmd - polecenie
    // buf - struktura do przechowywania informacji o pamieci wspoldzielonej
    // uzywamy tego by usunac pamiec wspoldzielona
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
