#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include "messeage.h"

bool flag = 1;

void handler(int signum){
    flag = 0;
}

int main(int argc, char* argv[]){

    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_msgsize = sizeof(message_msg),
        .mq_maxmsg = 10
    };

    mqd_t server_open_descriptor = mq_open(SERVER_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, &attr);
    if(server_open_descriptor == -1){
        printf("nie udalo sie otworzyc kolejki servera\n");
        perror("mq_open");
        exit(1);
    }

    message_msg recive_msg;

    mqd_t clients[10];

    for(int i = 0; i < 10; i++){
        clients[i] = -1;
    }

    signal(SIGINT, handler);

    while(flag){
        mq_receive(server_open_descriptor, (char*)&recive_msg, sizeof(recive_msg), NULL);
        switch (recive_msg.type)
        {
        case INIT:
            int id = 0;
            while(clients[id] != -1){
                id++;
            }
            // structura do wyslania do klienta
            message_msg rmsg_to_send ={
                .type = IDENTIFIER,
                .client_id = id
            };
            
            clients[id] = mq_open(recive_msg.text, O_RDWR, S_IRUSR | S_IWUSR, NULL);
            if(clients[id] == -1){
                printf("nie udalo sie otworzyc kolejki klienta\n");
                perror("mq_open");
                exit(1);
            }
            mq_send(clients[id], (char*)&rmsg_to_send, sizeof(message_msg), 1);
            printf("Klient %d dolaczyl do serwera\n", id);
            break;
    
        case MESSAGE:
            for(int i = 0; i < 10; i++){
                if(clients[i] != -1 && i != recive_msg.client_id){
                    mq_send(clients[i], (char*)&recive_msg, sizeof(recive_msg), 1);
                }
            }
            break;
        
        case STOP:
            if (clients[recive_msg.client_id] != -1) {
                message_msg end_msg = {
                    .type = STOP,
                    .client_id = recive_msg.client_id
                };
                char goodbye[] = "Dziekujemy za skorzystanie z naszych uslug, do zobaczenia!";
                memcpy(end_msg.text, goodbye, strlen(goodbye));

                if (mq_send(clients[recive_msg.client_id], (char*)&end_msg, sizeof(message_msg), 1) == -1) {
                    printf("nie udalo sie wyslac komunikatu konczącego\n");
                    perror("mq_send");
                    exit(1);
                }

                mq_close(clients[recive_msg.client_id]);
                clients[recive_msg.client_id] = -1;
                printf("Klient %d opuscil serwer\n", recive_msg.client_id);
            }
            break;
        }
    }
    mq_close(server_open_descriptor);
    mq_unlink(SERVER_NAME);
    for(int i = 0; i < 10; i++){
        if(clients[i] != -1){
            
            mq_close(clients[i]);
        }
    }
    printf("Zamknieto kolejki i ja tez sie zamykam by server!\n");
    return 0;
}
