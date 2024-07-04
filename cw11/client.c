#define XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define BUFFER_SIZE 256

int client_socket_fd;

void sigint_handler(int sig) {
    send(client_socket_fd, "STOP", 4, 0);
    close(client_socket_fd);
    exit(0);
}

void send_message(char *message) {
    send(client_socket_fd, message, strlen(message), 0);
}

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int bytes_received = recv(client_socket_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Server disconnected\n");
            break;
        }

        buffer[bytes_received] = '\0';
        if (strcmp(buffer, "ALIVE") == 0) {
            send(client_socket_fd, "ALIVE_response", 14, 0);
            continue;
        }


        printf("%s\n", buffer);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    char *client_name = argv[1];
    char *server_address = argv[2];
    struct sockaddr_in server_addr;

    client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket_fd < 0) {
        perror("Error opening client socket");
        return 1;
    }

    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[3]));

    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(client_socket_fd);
        return 1;
    }

    if (connect(client_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket_fd);
        return 1;
    }

    send(client_socket_fd, client_name, strlen(client_name), 0);

    signal(SIGINT, sigint_handler);

    pthread_t thread;
    pthread_create(&thread, NULL, receive_messages, NULL);

    char buffer[BUFFER_SIZE];
    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        send_message(buffer);
    }

    close(client_socket_fd);
    return 0;
}
