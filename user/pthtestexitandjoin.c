#include "lib.h"

int a[2] = {15,22};

void *add(void * a) {
    int * x = (int *) a;
    int res = x[0] + x[1];
    writef("res : %d\n", res);
    pthread_exit((void *) res);
}

void umain() {
    writef("===========================================\n");
    pthread_t thread;
    int iret;
    iret = pthread_create(&thread, NULL, add, (void *) a);
    int ret_val;
    pthread_join(thread, &ret_val);
    writef("return val : %d\n", ret_val);
    return;
}