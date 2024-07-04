#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/wait.h>
#include "messeage.h"

volatile bool flag = 1;

void handler(int signum){
    flag = 0;
}

int main(int argc, char* argv[]){

    // pobiera PID procesu
    pid_t PID1 = getpid();
    char client_queue_name[CLIENT_NAME_SIZE];
    // sprintf(char *str, const char *format, ...)
    // zapisuje sformatowany string do str
    sprintf(client_queue_name, "/client_queue_%d\n", PID1);

    // tworzenie kolejki komunikatow
    struct mq_attr attr = {
        .mq_flags = 0, // sygnalizator kolejki: 0, O_NONBLOCK
        .mq_msgsize = sizeof(message_msg), // max rozmiar komunikatu
        .mq_maxmsg = 10 // max liczba komunikatów w kolejce
        // long mq_curmsgs - liczba komunikatów w kolejce
    };

    // mqd_t mq_open(nazwa, flagi otwarcia, prawa dostepu, atrybuty)
    mqd_t client_que_descriptor = mq_open(client_queue_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, &attr);
    // funkcja probuje otowrzyc kolejke komunikatow (ktora tak naoprawde jest plikiem o nazwie podanej w argumencie)
    // zwraca deskryptor kolejki lub -1 w przypadku bledu, nazwa pliku musi zaczynac sie od / !!!

    if(client_que_descriptor == -1){
        printf("nie udalo sie otworzyc kolejki\n");
        perror("mq_open");
        exit(1);
    }

    // struktura do wyslania do servera
    message_msg msg = {
        .type = INIT,
        .client_id = -1,
    };

    // void *memcpy(void *dest, const void *src, size_t n)
    // kopiuje n bajtow z src do dest
    // uzywamy tego zamiast strcpy bo strcpy kopiuje do pierwszego napotkanego znaku \0
    memcpy(msg.text, client_queue_name, strlen(client_queue_name));
    printf(msg.text);

    // mqd_t mq_open(nazwa, flagi otwarcia, prawa dostepu, atrybuty)
    // otwiera kolejke servera
    mqd_t server_open_descriptor = mq_open(SERVER_NAME, O_RDWR, S_IRUSR | S_IWUSR, NULL);
    if(server_open_descriptor == -1){
        printf("nie udalo sie otworzyc kolejki servera\n");
        perror("mq_open");
        exit(1);
    }  

    // mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio)
    // wysyla komunikat o dlugosci msg_len z msg_ptr do kolejki o deskryptorze mqdes
    if(mq_send(server_open_descriptor, (char*)&msg, sizeof(msg), 1) == -1){
        printf("nie udalo sie wyslac komunikatu\n");
        perror("mq_send");
        exit(1);
    }

    int fd[2];
    if(pipe(fd) == -1){
        printf("nie udalo sie utworzyc potoku\n");
        perror("pipe");
        exit(1);
    }

    // signal(int signum, void (*handler)(int))
    // ustawia handler na sygnal signum
    signal(SIGINT, handler);

    int client_id;

    // pid_t fork(void)
    // tworzy proces potomny
    pid_t PID = fork();
    if(PID == -1){
        printf("nie udalo sie utworzyc procesu potomnego\n");
        perror("fork");
        exit(1);
    } else if(PID == 0){
        close(fd[0]);
        // struktura do odbierania komunikatu
        message_msg recive_msg;
        while(flag){
            // mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio)
            // odbiera komunikat z kolejki o deskryptorze mqdes i zapisuje go do msg_ptr
            // gdy odbierzemy komunikat z kolejki to zwraca jego dlugosc i jest on z kolejki usuwany
            int received = mq_receive(client_que_descriptor, (char*)&recive_msg, sizeof(message_msg), NULL);
            if(received != -1) {
                switch(recive_msg.type){
                    case IDENTIFIER:
                        // write(int fd, const void *buf, size_t count)
                        // pisze przez potok do procesu macierzystego
                        write(fd[1], &recive_msg.client_id, sizeof(recive_msg.client_id));
                        printf("Otrzymano identyfikator: %d\n", recive_msg.client_id);
                        break;
                    case MESSAGE:
                        printf("otrzymano widomosc od %d: %s\n", recive_msg.client_id, recive_msg.text);
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
        while (flag)
        {
            // %ms - alokuje pamiec na stringa i zapisuje go do zmiennej
            if(scanf("%ms", &buffer)){
                // structura do wyslania do servera
                message_msg new_msg = {
                    .type = MESSAGE,
                    .client_id = client_id
                };
                // void *memcpy(void *dest, const void *src, size_t n)
                // kopiuje n bajtow z src do dest
                // uzywamy tego zamiast strcpy bo strcpy kopiuje do pierwszego napotkanego znaku \0
                memcpy(new_msg.text, buffer, strlen(buffer));

                // mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio)
                // wysyla komunikat o dlugosci msg_len z msg_ptr do kolejki o deskryptorze mqdes
                // msg_prio - priorytet komunikatu, 0 najnizszy, 31 najwyzszy
                if(mq_send(server_open_descriptor, (char*)&new_msg, sizeof(new_msg), 1) == -1){
                    printf("nie udalo sie wyslac komunikatu\n");
                    perror("mq_send");
                    exit(1);
                }
                // zwalnianie pamieci
                free(buffer);
                buffer = NULL;
            }
        }
    }

    message_msg end_msg = {
        .type = STOP,
        .client_id = client_id
    };

    if(mq_send(server_open_descriptor, (char*)&end_msg, sizeof(end_msg), 1) == -1){
        printf("nie udalo sie wyslac komunikatu konczonczego\n");
        perror("mq_send");
        exit(1);
    }

    waitpid(PID, NULL, 0);

    // zamkniecie kolejek
    mq_close(client_que_descriptor);
    mq_close(server_open_descriptor);

    // usuniecie kolejek
    mq_unlink(client_queue_name);
    close(fd[0]);


    // mq_getattr(mqd_t mqdes, struct mq_attr *attr)
    // odczytuje atrybuty kolejki komunikatow o deskryptorze mqdes i zapusuje je do struktury attr
    // przydaje sie to gdy chcemy sprawdzic ile komunikatow jest w kolejce

    // mq_setattr(mqd_t mqdes, const struct mq_attr *newattr, struct mq_attr *oldattr)
    // ustawia nowe atrybuty kolejki komunikatow o deskryptorze mqdes na te podane w newattr
    // stare atrybuty sa zapisywane w oldattr, przydaje sie to gdy chcemy zmienic atrybuty kolejki

    // mq_notify(mqd_t mqdes, const struct sigevent *notification)
    // ustawia powiadomienie o nowym komunikacie w kolejce o deskryptorze mqdes
    // notification - struktura z informacjami o powiadomieniu
    // przydaje sie to gdy chcemy zareagowac na nowy komunikat w kolejce

}
