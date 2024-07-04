#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "buffer.h"
#include <asm-generic/fcntl.h>
#include <sys/mman.h>
#include <string.h>

#define M 3 

int main() {
    Buffer *buffer;
    // tworzenie pamieci wspoldzielonej
    // nazwa pamieci wspoldzielonej w pliku naglowkowym
    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // zmiana rozmiaru pamieci wspoldzielonej
    // robi sie to po to aby pamiec wspoldzielona miala rozmiar struktury Buffer
    if (ftruncate(shm_fd, sizeof(Buffer)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    // mapowanie pamieci wspoldzielonej
    // robi sie to zawsze po zmianie rozmiaru pamieci wspoldzielonej
    // otwarty segment pamieci wspoldzielonej jest mapowany do przestrzeni adresowej procesu
    // mmap( void *addr, size_t length, int prot, int flags, int fd, off_t offset)
    // addr - adres poczatkowy mapowania, NULL - system wybiera adres   
    // length - dlugosc mapowania
    // prot - okresla prawa dostepu do mapowanego obszaru
    // MAP_SHARED - zmiany dokonane w obszarze mapowanym sa widoczne w pamieci wspoldzielonej
    // MAP_PRIVATE - zmiany dokonane w obszarze mapowanym nie sa widoczne w pamieci wspoldzielonej
    // MAP_FIXED - wymusza mapowanie na okreslony adres
    // fd - deskryptor pliku
    // offset - przesuniecie w pliku
    buffer = mmap(NULL, sizeof(Buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (buffer == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // inicjalizacja semaforow
    // sem_init(sem_t *sem, int pshared, unsigned int value)
    // sem - wskaznik do semafora
    // pshared - okresla czy semafor jest wspoldzielony miedzy procesami
    // value - wartosc poczatkowa semafora
    // wskazniki sa w pliku naglowkowym
    if (sem_init(&(buffer->empty), 1, BUFFER_SIZE) == -1 ||
        sem_init(&(buffer->full), 1, 0) == -1 ||
        sem_init(&(buffer->used), 1, 1) == -1) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    buffer->in = 0;
    buffer->out = 0;

    // jest to po to by sie lepiej wypisywalo
    setbuf(stdout, NULL);

    srand(time(NULL));

    for (int i = 0; i < M; ++i) {
        pid_t pid = fork();
        if (pid == 0) { 
            while (1) {
                // sem_wait(sem_t *sem)
                // zmniejsza wartosc semafora o 1
                // jesli wartosc semafora jest rowna 0 to proces czeka na zwiekszenie wartosci
                sem_wait(&(buffer->full));
                sem_wait(&(buffer->used));

                char task[11];
                strcpy(task, buffer->tasks[buffer->out]);
                buffer->out = (buffer->out + 1) % BUFFER_SIZE;

                // sem_post(sem_t *sem)
                // zwieksza wartosc semafora o 1
                // odblokowuje procesy czekajace na semaforze
                sem_post(&(buffer->used));
                sem_post(&(buffer->empty));

                //printf("Drukarka %d: , znak: %s \n", getpid(), task);
                //sleep(2);

                for (int i = 0; i < 11; ++i) {
                    if (task[i] != '\0')
                        printf("Drukarka %d: , znak: %c \n", getpid(), task[i]);
                    else
                        break;
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

    // sem_destroy(sem_t *sem)
    // usuwa semafor
    if (sem_destroy(&(buffer->empty)) == -1 ||
        sem_destroy(&(buffer->full)) == -1 ||
        sem_destroy(&(buffer->used)) == -1) {
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }

    return 0;
}
