#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include "messeage.h"

bool flag = 1;

void handler(int signum) {
    flag = 0;
}

int main(int argc, char* argv[]) {
    key_t key = ftok(".", 'S');
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    message_msg recive_msg;

    mqd_t clients[10];

    for (int i = 0; i < 10; i++) {
        clients[i] = -1;
    }

    signal(SIGINT, handler);

    while (flag) {
        if (msgrcv(msgid, &recive_msg, sizeof(recive_msg) - sizeof(long), 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        switch (recive_msg.type) {
        case INIT: {
            int id = 0;
            while (clients[id] != -1) {
                id++;
            }

            message_msg rmsg_to_send = {
                .type = IDENTIFIER,
                .client_id = id
            };

            key_t client_key = ftok(recive_msg.text, 'C');
            clients[id] = msgget(client_key, 0666);
            if (clients[id] == -1) {
                perror("msgget client");
                exit(1);
            }

            if (msgsnd(clients[id], &rmsg_to_send, sizeof(message_msg) - sizeof(long), 0) == -1) {
                perror("msgsnd");
                exit(1);
            }
            printf("Klient %d dolaczyl do serwera\n", id);
            break;
        }

        case MESSAGE: {
            for (int i = 0; i < 10; i++) {
                if (clients[i] != -1 && i != recive_msg.client_id) {
                    if (msgsnd(clients[i], &recive_msg, sizeof(message_msg) - sizeof(long), 0) == -1) {
                        perror("msgsnd");
                        exit(1);
                    }
                }
            }
            break;
        }

        case STOP: {
            if (clients[recive_msg.client_id] != -1) {
                message_msg end_msg = {
                    .type = STOP,
                    .client_id = recive_msg.client_id
                };
                char goodbye[] = "Dziekujemy za skorzystanie z naszych uslug, do zobaczenia!";
                memcpy(end_msg.text, goodbye, strlen(goodbye));

                if (msgsnd(clients[recive_msg.client_id], &end_msg, sizeof(message_msg) - sizeof(long), 0) == -1) {
                    perror("msgsnd");
                    exit(1);
                }

                clients[recive_msg.client_id] = -1;
                printf("Klient %d opuscil serwer\n", recive_msg.client_id);
            }
            break;
        }
        }
    }

    msgctl(msgid, IPC_RMID, NULL);
    printf("Zamknieto kolejki i ja tez sie zamykam by server!\n");
    return 0;
}
