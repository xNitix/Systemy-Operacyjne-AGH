#ifndef MESSEAGE_H
#define MESSEAGE_H
#define MAX_MESSAGE_SIZE 4096
#define SERVER_NAME "/server_name"
#define CLIENT_NAME_SIZE 30

typedef enum {
    INIT, 
    IDENTIFIER,
    MESSAGE,
    STOP
} message_type;

// struktura komunikatu
typedef struct{
    message_type type;
    int client_id;
    char text[MAX_MESSAGE_SIZE];
}message_msg;

#endif //PROTOCOL_SPECS_H