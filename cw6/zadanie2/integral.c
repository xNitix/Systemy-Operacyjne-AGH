#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

float function(float x){
    return 4/(x*x+1);
}

float intergral(float start, float end, float step){
    float result = 0;
    while (start + step < end){
        result += function((start + step)/2) * step;
        start += step;
    }
    result += function((end-start)/2+start) * (end-start);
    return result;
}

int main(int argc, char *argv[]){
    int fifo = open("fifoInput", O_RDONLY);
    int fifo2 = open("fifoOutput", O_WRONLY);

    float start, end;
    read(fifo, &start, sizeof(start));
    read(fifo, &end, sizeof(end));

    int n = 10;
    float res = 0;
    float step = n/(end-start);
    for(int i = 0; i < n; i++){
        res += intergral(i*step, (i+1)*step, step);
    }
    write(fifo2, &res, sizeof(res));

    close(fifo);
    close(fifo2);
    
    return 0;
}