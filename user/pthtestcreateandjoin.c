//
// Created by root on 6/28/22.
//

#include "lib.h"

void *print(void * c) {
    writef("^^^ %s\n", c);
}

void umain() {
    writef("===========================================\n");
    pthread_t pthread1, pthread2;
    char *message1 = "This is Thread 1";
    char *message2 = "This is Thread 2";
    int iret1, iret2;
    writef("umain envid : %d\n", syscall_getenvid());
    iret1 = pthread_create(&pthread1, NULL, print, (void *) message1);
    iret2 = pthread_create(&pthread2, NULL, print, (void *) message2);
    writef("finish pthread_create\n");
    pthread_join(pthread1, NULL);
    pthread_join(pthread2, NULL);
    writef("ret1 : %d\n", iret1);
    writef("ret2 : %d\n", iret2);
    return;
}
