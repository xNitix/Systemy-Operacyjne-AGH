#include <stdio.h>

#ifndef DYNAMIC
    #include "collatz.h"
#endif

#ifdef DYNAMIC
    #include <dlfcn.h>
    int (*test_collatz_convergence)(int input, int max_iter);
#endif

int main() {
    int input;
    int max_iter;
    int result;

    printf("Enter a number: ");
    scanf("%d", &input);
    printf("Enter the maximum number of iterations: ");
    scanf("%d", &max_iter);

    #ifdef DYNAMIC
        void *handle = dlopen("./libcollatz.so", RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "%s\n", dlerror());
            return -1;
        }

        test_collatz_convergence = dlsym(handle, "test_collatz_convergence");
        
        if (dlerror() != NULL) {
            fprintf(stderr, "%s\n", dlerror());
            return -1;
        }
    #endif

    result = test_collatz_convergence(input, max_iter);
    if(result == -1){
        printf("result: faild\n");
    } else {
        printf("result: %d \n", result);
    }

    return 0;

    #ifdef DYNAMIC
        dlclose(handle);
    #endif

}