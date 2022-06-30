#include "lib.h"
#include <semaphore.h>

void sem_init(sem_t * sem, u_int pshared, u_int init_val) {
    *sem = syscall_sem_init(pshared, init_val);
    return;
}

void sem_destroy(sem_t sem) {
    syscall_sem_destroy(sem);
}

int sem_trywait(sem_t sem) {
    return syscall_sem_trywait(sem);
}

void sem_wait(sem_t sem) {
    syscall_sem_wait(sem);
}

void sem_post(sem_t sem) {
    syscall_sem_post(sem);
}

int sem_getvalue(sem_t sem) {
    return syscall_sem_getvalue(sem);
}