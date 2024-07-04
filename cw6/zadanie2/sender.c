#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[]){
    float Istart = atof(argv[1]);
    float Iend = atof(argv[2]);

    mkfifo("fifoInput", 0777);
    mkfifo("fifoOutput", 0777);
    int fifo = open("fifoInput", O_WRONLY);
    write(fifo, &Istart, sizeof(Istart));
    write(fifo, &Iend, sizeof(Iend));

    float result;
    int fifo2 = open("fifoOutput", O_RDONLY);
    read(fifo2, &result, sizeof(result));
    printf("Wynik obliczeń: %f\n", result);
    close(fifo);
    close(fifo2);

    return 0;
}