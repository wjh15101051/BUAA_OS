#include "lib.h"

void thread2func(void) {
    writef("This is Thread 2\n");
}

void thread1func(void) {
    pthread_t thread2;
    int iret2;
    iret2 = pthread_create(&thread2, NULL, thread2func, NULL);
    int ret_val2;
    pthread_join(thread2, &ret_val2);
    writef("thread2 return val : %d\n", ret_val2);
}

void umain() {
    pthread_t thread1;
    int iret1;
    iret1 = pthread_create(&thread1, NULL, thread1func, NULL);
    int ret_val1;
    pthread_join(thread1, &ret_val1);
    writef("thread1 return val : %d\n", ret_val1);
    return;
}