#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 256

typedef struct {
    int socket_fd;
    char name[50];
    pthread_t thread;
    int active;
    pthread_mutex_t recv_mutex; 
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];
int num_clients = 0;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_to_all(char *message, char *sender_name) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].active && strcmp(clients[i].name, sender_name) != 0) {
            char buffer[BUFFER_SIZE];
            time_t rawtime = time(NULL);
            char* date = ctime(&rawtime);
            sprintf(buffer,date);
            strcat(buffer, " ");
            strcat(buffer, sender_name);
            strcat(buffer, ": ");
            strcat(buffer, message);
            send(clients[i].socket_fd, buffer, strlen(buffer), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_to_one(char *message, char *sender_name, char *recipient_name) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].active && strcmp(clients[i].name, recipient_name) == 0) {
            char buffer[BUFFER_SIZE];
            time_t rawtime = time(NULL);
            char* date = ctime(&rawtime);
            sprintf(buffer,date);
            strcat(buffer, " ");
            strcat(buffer, sender_name);
            strcat(buffer, ": ");
            strcat(buffer, message);
            send(clients[i].socket_fd, buffer, strlen(buffer), 0);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *client_handler(void *arg) {
    int client_index = *((int *)arg);
    char client_name[50];
    strcpy(client_name, clients[client_index].name);

    while (1) {
        char buffer[BUFFER_SIZE];
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        setsockopt(clients[client_index].socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof timeout);
        pthread_mutex_lock(&clients[client_index].recv_mutex);
        int bytes_received = recv(clients[client_index].socket_fd, buffer, BUFFER_SIZE, 0);
        pthread_mutex_unlock(&clients[client_index].recv_mutex);

        if (clients[client_index].active == 0) {
            printf("Client %s disconnected\n", clients[client_index].name);
            break;
        }

        if(bytes_received == -1){
            continue;
        }

        buffer[bytes_received] = '\0';

        if (strncmp(buffer, "2ALL ", 5) == 0) {
            send_to_all(buffer + 5, client_name);
        } else if (strncmp(buffer, "2ONE ", 5) == 0) {
            char recipient_name[50];
            sscanf(buffer + 5, "%s", recipient_name);
            send_to_one(buffer + 6 + strlen(recipient_name), client_name, recipient_name);
        } else if (strncmp(buffer, "LIST", 4) == 0) {
            char list_message[BUFFER_SIZE];
            strcpy(list_message, "Clients on server : \n");
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (clients[i].active) {
                    strcat(list_message, clients[i].name);
                    strcat(list_message, "\n");
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            send(clients[client_index].socket_fd, list_message, strlen(list_message), 0);
        } else if (strncmp(buffer, "STOP", 4) == 0) {
            printf("Client %s requested to stop\n", clients[client_index].name);
            close(clients[client_index].socket_fd);
            pthread_mutex_lock(&clients_mutex);
            clients[client_index].active = 0;
            num_clients--;
            pthread_mutex_unlock(&clients_mutex);
            break;
        }
    }
    pthread_exit(NULL);
}

void *check_alive(void *arg) {
    int client_index = *((int *)arg);
    free(arg);
    char buffer[BUFFER_SIZE];

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(clients[client_index].socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,sizeof timeout);

    pthread_mutex_lock(&clients[client_index].recv_mutex);
    send(clients[client_index].socket_fd, "ALIVE", 5, 0);
    int bytes_received = read(clients[client_index].socket_fd, buffer, BUFFER_SIZE);
    pthread_mutex_unlock(&clients[client_index].recv_mutex);

    if (bytes_received <= 0) {
        close(clients[client_index].socket_fd);
        pthread_mutex_lock(&clients_mutex);
        clients[client_index].active = 0;
        num_clients--;
        pthread_mutex_unlock(&clients_mutex);
    } 
    pthread_exit(NULL);
}

void *alive_checker(void *arg) {
    while (1) {
        sleep(10);
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i].active) {
                int *client_index = malloc(sizeof(int));
                *client_index = i;
                pthread_t thread;
                pthread_create(&thread, NULL, check_alive, client_index);
                pthread_detach(thread);
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
}

int main(int argc, char *argv[]) {
    int server_socket_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0) {
        perror("Error opening server socket");
        return 1;
    }

    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding server socket");
        close(server_socket_fd);
        return 1;
    }

    listen(server_socket_fd, 10);

    pthread_t alive_thread;
    pthread_create(&alive_thread, NULL, alive_checker, NULL);

    printf("Chat is alive, waiting for clients\n");

    while (1) {
        int client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket_fd < 0) {
            perror("Error accepting client connection");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (num_clients >= MAX_CLIENTS) {
            pthread_mutex_unlock(&clients_mutex);
            close(client_socket_fd);
            continue;
        }

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (!clients[i].active) {
                clients[i].socket_fd = client_socket_fd;
                recv(client_socket_fd, clients[i].name, 50, 0);
                clients[i].active = 1;
                pthread_mutex_init(&clients[i].recv_mutex, NULL); 
                num_clients++;
                pthread_create(&clients[i].thread, NULL, client_handler, &i);
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    close(server_socket_fd);
    return 0;
}
