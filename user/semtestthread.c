#include "lib.h"
#include <semaphore.h>

sem_t mutex;

void *work(void * c) {
    writef("^^^ enter %s\n", c);
    if (sem_trywait(mutex) == TRYWAIT_SUCCEED) {
        writef("%s got mutex in try wait\n", c);
        int i;
        for (i = 0; i < 10; ++i) {
            syscall_yield();
        }
        sem_post(mutex);
        writef("%s release mutex\n", c);
    } else {
        writef("%s didn't get mutex in wait\n", c);
        sem_wait(mutex);
        writef("%s got mutex after wait\n", c);
    }
}

void umain() {
    writef("===========================================\n");
    sem_init(&mutex, 0, 1);
    pthread_t pthread1, pthread2;
    char *message1 = "Thread 1";
    char *message2 = "Thread 2";
    int iret1, iret2;
    writef("umain envid : %d\n", syscall_getenvid());
    iret1 = pthread_create(&pthread1, NULL, work, (void *) message1);
    iret2 = pthread_create(&pthread2, NULL, work, (void *) message2);
    writef("finish pthread_create\n");
    pthread_join(pthread1, NULL);
    pthread_join(pthread2, NULL);
    writef("ret1 : %d\n", iret1);
    writef("ret2 : %d\n", iret2);
    sem_destroy(mutex);
}