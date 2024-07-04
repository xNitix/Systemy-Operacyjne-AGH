#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "buffer.h"
#include <asm-generic/fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/time.h>

#define N 5 
int main() {
    Buffer *buffer; 
    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, sizeof(Buffer)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    
    buffer = mmap(NULL, sizeof(Buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (buffer == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    setbuf(stdout, NULL);

    for (int i = 0; i < N; ++i) {
        pid_t pid = fork();
        if (pid == 0) { 
            while (1) {
                srand(getpid() * time(NULL));
                sem_wait(&(buffer->empty));
                sem_wait(&(buffer->used));

                char task[11];
                for (int i = 0; i < 10; ++i) {
                    task[i] = 'a' + rand() % 26;  
                }
                printf("PID: %d zlecilem wydruk: %s\n",getpid(),task);

                task[10] = '\0'; 
                strcpy(buffer->tasks[buffer->in], task);
                buffer->in = (buffer->in + 1) % BUFFER_SIZE;

                sem_post(&(buffer->used));
                sem_post(&(buffer->full));

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
