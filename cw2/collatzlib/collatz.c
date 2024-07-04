#include <stdio.h>
#include "collatz.h"

int collatz_conjecture(int input) {
    if (input % 2 == 0) {
        return input / 2;
    } else {
        return (3 * input) + 1;
    }
}

int test_collatz_convergence(int input, int max_iter) {
    int iter = 0;

    while (input != 1 && iter < max_iter) {
        iter++;
        input = collatz_conjecture(input);
    }

    if(input == 1){
        return iter;
    } else {
        return -1;
    }
    
}