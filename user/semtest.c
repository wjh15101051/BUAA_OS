#include "lib.h"
#include <semaphore.h>

void umain() {
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    writef("===========================================\n");
    int r;

    if (fork() == 0) {
        // child
        writef("enter child\n");
        if (sem_trywait(mutex) == TRYWAIT_SUCCEED) {
            writef("child got mutex in try wait\n");
            int i;
            for (i = 0; i < 10; ++i) {
                syscall_yield();
            }
            sem_post(mutex);
            writef("child release mutex\n");
        } else {
            writef("child didn't get mutex in wait\n");
            sem_wait(mutex);
            writef("child got mutex after wait\n");
        }
    } else {
        // parent
        writef("enter parent\n");
        if ((r = sem_trywait(mutex)) == TRYWAIT_SUCCEED) {
            writef("parent got mutex in try wait\n");
            int i;
            for (i = 0; i < 10; ++i) {
                syscall_yield();
            }
            sem_post(mutex);
            writef("parent release mutex\n");
        } else {
            writef("parent didn't get mutex in wait\n");
            writef("r : %d\n", r);
            sem_wait(mutex);
            writef("parent got mutex after wait\n");
        }
    }
    sem_destroy(mutex);
}