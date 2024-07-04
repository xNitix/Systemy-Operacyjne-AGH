#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

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
    float step = atof(argv[1]);
    int n = atoi(argv[2]);

    __clock_t start = clock();
    float res = 0;
    int data[n];
    float step1 = 1.0/n;
    for(int i = 0; i < n; i++){
        int fd[2];
        pipe(fd);

        if(fork() == 0){
            close(fd[0]);
            float result = intergral(i*step1, (i+1)*step1, step);
            write(fd[1], &result, sizeof(result));
            close(fd[1]);
            exit(0);
        } 
        else{
            close(fd[1]);
            data[i] = fd[0];
        }
    }

    while(wait(NULL) >= 0){}; 

    for(int i = 0; i < n; i++){
        float temp;
        read(data[i], &temp, sizeof(temp));
        res += temp;
    }

    __clock_t end = clock();
    __clock_t time = end - start;

    printf("Wynik: %f\n", res);
    printf("Czas: %f\n", (double)time/CLOCKS_PER_SEC);
    return 0;
}