#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/wait.h>

#include "messeage.h"

volatile bool flag = 1;

void handler(int signum) {
    flag = 0;
}

int main(int argc, char* argv[]) {
    pid_t PID1 = getpid();
    char client_queue_name[CLIENT_NAME_SIZE];
    sprintf(client_queue_name, "client_queue_%d", PID1);

    key_t client_key = ftok(client_queue_name, 'C');
    // int msgget(key_t key, int msgflg)
    // tworzy nowa kolejke komunikatow lub otwiera istniejaca
    // zwraca identyfikator kolejki lub -1 w przypadku bledu
    int client_msgid = msgget(client_key, IPC_CREAT | 0666);
    if (client_msgid == -1) {
        perror("msgget client");
        exit(1);
    }

    message_msg msg = {
        .type = INIT,
        .client_id = -1
    };
    memcpy(msg.text, client_queue_name, strlen(client_queue_name) + 1);

    // key_t ftok(const char *pathname, int proj_id)
    // generuje klucz IPC na podstawie sciezki do pliku i id projektu
    key_t server_key = ftok(".", 'S');
    // int msgget(key_t key, int msgflg)
    // tworzy nowa kolejke komunikatow lub otwiera istniejaca
    int server_msgid = msgget(server_key, 0666);
    if (server_msgid == -1) {
        perror("msgget server");
        exit(1);
    }

    // int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
    // wysyla komunikat do kolejki
    if (msgsnd(server_msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    signal(SIGINT, handler);

    int client_id;

    pid_t PID = fork();
    if (PID == -1) {
        perror("fork");
        exit(1);
    } else if (PID == 0) {
        close(fd[0]);
        message_msg recive_msg;
        while (flag) {
            // ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
            // odbiera komunikat z kolejki
            // zwraca dlugosc komunikatu lub -1 w przypadku bledu
            if (msgrcv(client_msgid, &recive_msg, sizeof(recive_msg) - sizeof(long), 0, 0) != -1) {
                switch (recive_msg.type) {
                    case IDENTIFIER:
                        write(fd[1], &recive_msg.client_id, sizeof(recive_msg.client_id));
                        printf("Otrzymano identyfikator: %d\n", recive_msg.client_id);
                        break;
                    case MESSAGE:
                        printf("Otrzymano wiadomość od %d: %s\n", recive_msg.client_id, recive_msg.text);
                        break;
                    case STOP:
                        printf("%s\n", recive_msg.text);
                        break;
                    default:
                        break;
                }
            }
        }
        close(fd[1]);
        exit(0);
    } else {
        close(fd[1]);
        read(fd[0], &client_id, sizeof(client_id));
        char* buffer = NULL;
        while (flag) {
            if (scanf("%ms", &buffer)) {
                message_msg new_msg = {
                    .type = MESSAGE,
                    .client_id = client_id
                };
                memcpy(new_msg.text, buffer, strlen(buffer) + 1);

                if (msgsnd(server_msgid, &new_msg, sizeof(new_msg) - sizeof(long), 0) == -1) {
                    perror("msgsnd");
                    exit(1);
                }
                free(buffer);
                buffer = NULL;
            }
        }
    }

    message_msg end_msg = {
        .type = STOP,
        .client_id = client_id
    };

    // int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
    // wysyla komunikat do kolejki
    if (msgsnd(server_msgid, &end_msg, sizeof(end_msg) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    waitpid(PID, NULL, 0);

    // int msgctl(int msqid, int cmd, struct msqid_ds *buf)
    // wykonuje operacje na kolejce komunikatow
    // IPC_RMID - usuwa kolejke
    msgctl(client_msgid, IPC_RMID, NULL);
    close(fd[0]);

    return 0;
}
