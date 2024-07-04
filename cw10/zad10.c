#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define REINDEERS_NUM 9
#define MAX_REINDEERS_IN_QUEUE 9
#define presents_delivery 4
int reindeers_in_queue = 0;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_for_reindeers = PTHREAD_COND_INITIALIZER;
pthread_cond_t condition_for_santa = PTHREAD_COND_INITIALIZER;

void* reindeer_mechanic(void *arg){
    (void)arg;
    while(1){
        // reindeers wacation
        sleep(rand() % 6 + 5);

        pthread_mutex_lock(&mutex);
        reindeers_in_queue++;
        printf("Renifer: czeka %d reniferów na Mikołaja\n", reindeers_in_queue);

        if(reindeers_in_queue == MAX_REINDEERS_IN_QUEUE){
            printf("Renifer: wybudzam Mikołaja\n");
            pthread_cond_signal(&condition_for_santa);
        }

        while(reindeers_in_queue > 0){
            pthread_cond_wait(&condition_for_reindeers, &mutex);
        }

        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* santa_mechanic(void *arg){
    (void)arg;
    int delivered_presents = 0;
    while(delivered_presents < presents_delivery){
        pthread_mutex_lock(&mutex);

        while(reindeers_in_queue < MAX_REINDEERS_IN_QUEUE){
            pthread_cond_wait(&condition_for_santa, &mutex);
        }

        printf("Mikołaj: budze się\n");

        time_t t = rand() % 3 + 2;
        for(int i = 0; i < t; i++){
            printf("Mikołaj: dostarczam zabawki\n");
            sleep(1);
        }

        printf("Mikołaj: zasypiam\n");
        delivered_presents++;
        reindeers_in_queue = 0;

        pthread_cond_broadcast(&condition_for_reindeers);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char* argv[]){
    (void)argc;
    (void)argv;
    pthread_t santa;
    pthread_t reindeers[REINDEERS_NUM];
    srand(time(NULL));

    if(pthread_create(&santa, NULL, santa_mechanic, NULL)){
        perror("Error while creating santa thread");
        exit(1);
    }

    for(int i = 0; i < REINDEERS_NUM; i++){
        if(pthread_create(&reindeers[i], NULL, reindeer_mechanic, NULL) != 0){
            perror("Error while creating reindeer thread");
            exit(1);
        }
    }

    if(pthread_join(santa, NULL)){
        perror("Error while joining santa thread");
        exit(1);
    }

    for(int i = 0; i < REINDEERS_NUM; i++){
       pthread_cancel(reindeers[i]);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition_for_reindeers);
    pthread_cond_destroy(&condition_for_santa);
    return 0;
}