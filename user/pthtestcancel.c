#include "lib.h"

void *subprocess(void) {
    int i;
    for (i = 0; i < 1000000; ++i) {
        writef("%d\n", i);
    }
}

void umain() {
    writef("===========================================\n");
    pthread_t thread;
    int iret;
    iret = pthread_create(&thread, NULL, subprocess, NULL);
    int i = 0;
    for (i = 0; i < 10000; ++i) {
        writef("%d\n", -i);
    }
    pthread_cancel(thread);
    return;
}