#include <semaphore.h>
#include <env.h>
#include <queue.h>
#include <sched.h>
#include <pthread.h>
#include <error.h>
#include <printf.h>

struct Sem_list sem_wait_list[SEMAPHORE_MAX];
struct Sem * sems;
int bitmap = 0;
int val[SEMAPHORE_MAX] = {0};
int sem_env[SEMAPHORE_MAX] = {0};

extern char *KERNEL_SP;

struct Sem* env2sem(struct Env * e) {
    return sems + (u_int) ENVX(e->env_id);
}

struct Env * sem2env(struct Sem * sem) {
    return envs + (u_int) (sem - sems);
}

int semaphore_alloc() {
    int i;
    for (i = 0; i < SEMAPHORE_MAX; ++i) {
        if ((bitmap & (1 << i)) == 0) {
            bitmap |= (1 << i);
            return i;
        }
    }
    return -1;
}

int sem_init(u_int pshared, u_int init_val) {
    int sem = semaphore_alloc();
    val[sem] = init_val;
    LIST_INIT(&sem_wait_list[sem]);
    if (pshared == 0) {
        u_int cur_env_id = (pthreads + (u_int) (curenv - envs))->env_id;
        if (cur_env_id == 0) {
            cur_env_id = curenv->env_id;
        }
        sem_env[sem] = cur_env_id;
    } else {
        sem_env[sem] = 0;
    }
    return sem;
}

void sem_destroy(sem_t sem) {
    u_int cur_env_id = (pthreads + (u_int) (curenv - envs))->env_id;
    if (cur_env_id == 0) {
        cur_env_id = curenv->env_id;
    }
    if (sem_env[sem] != 0 && cur_env_id != sem_env[sem]) {
        panic("wrong env !!!!!!!!!!!\n");
    }
    bitmap ^= (1 << sem);
    return;
}

int sem_trywait(sem_t sem) {
    u_int cur_env_id = (pthreads + (u_int) (curenv - envs))->env_id;
    if (cur_env_id == 0) {
        cur_env_id = curenv->env_id;
    }
    if (sem_env[sem] != 0 && cur_env_id != sem_env[sem]) {
        panic("wrong env !!!!!!!!!!!\n");
    }
    if (val[sem] > 0) {
        --val[sem];
        ++(env2sem(curenv)->have[sem]);
        return TRYWAIT_SUCCEED;
    } else {
        return TRYWAIT_FAILED;
    }
}

void sem_wait(sem_t sem) {
    u_int cur_env_id = (pthreads + (u_int) (curenv - envs))->env_id;
    if (cur_env_id == 0) {
        cur_env_id = curenv->env_id;
    }
    if (sem_env[sem] != 0 && cur_env_id != sem_env[sem]) {
        panic("wrong env !!!!!!!!!!!\n");
    }
    struct Sem * envsem = env2sem(curenv);
    if (val[sem] > 0) {
        --val[sem];
        ++(env2sem(curenv)->have[sem]);
    } else {
        LIST_INSERT_TAIL(&sem_wait_list[sem], envsem, sem_link);
        curenv->env_status = ENV_NOT_RUNNABLE;
        size_t s = sizeof(struct Trapframe);
        bcopy((void *) KERNEL_SP - s, (void *) TIMESTACK - s, s);
        sched_yield();
    }
    return;
}

void sem_post(sem_t sem) {
    u_int cur_env_id = (pthreads + (u_int) (curenv - envs))->env_id;
    if (cur_env_id == 0) {
        cur_env_id = curenv->env_id;
    }
    if (sem_env[sem] != 0 && cur_env_id != sem_env[sem]) {
        panic("wrong env !!!!!!!!!!!\n");
    }
    struct Sem * envsem = env2sem(curenv);
    --envsem->have[sem];
    if (!LIST_EMPTY(&sem_wait_list[sem])) {
        struct Sem * getsem = LIST_FIRST(&sem_wait_list[sem]);
        LIST_REMOVE(getsem, sem_link);
        --val[sem];
        ++getsem->have[sem];
        sem2env(getsem)->env_status = ENV_RUNNABLE;
    }
}

int sem_getvalue(sem_t sem) {
    u_int cur_env_id = (pthreads + (u_int) (curenv - envs))->env_id;
    if (cur_env_id == 0) {
        cur_env_id = curenv->env_id;
    }
    if (sem_env[sem] != 0 && cur_env_id != sem_env[sem]) {
        panic("wrong env !!!!!!!!!!!\n");
    }
    return val[sem];
}