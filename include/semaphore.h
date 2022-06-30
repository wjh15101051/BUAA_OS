#ifndef OS_SEMAPHORE_H
#define OS_SEMAPHORE_H

#include <queue.h>
#include <env.h>

typedef u_int sem_t;

#define SEMAPHORE_MAX 30

#define TRYWAIT_SUCCEED 1
#define TRYWAIT_FAILED 2

struct Sem {
    LIST_ENTRY(Sem) sem_link;
    u_int have[SEMAPHORE_MAX];
};

LIST_HEAD(Sem_list, Sem);

extern struct Sem * sems;

#endif //OS_SEMAPHORE_H
