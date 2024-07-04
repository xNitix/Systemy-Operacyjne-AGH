#ifndef BUFFER_H
#define BUFFER_H

#include <semaphore.h>

#define BUFFER_SIZE 10
// pamieć współdzielona 
#define SHARED_MEMORY_NAME "/shared_buffer"

typedef struct {
    char tasks[BUFFER_SIZE][11]; 
    int in;
    int out;
    sem_t empty;
    sem_t full;
    sem_t used;
} Buffer;

#endif /* BUFFER_H */
