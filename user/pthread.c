#include "lib.h"
#include <pthread.h>
#include <printf.h>
#include <mmu.h>
#include <env.h>

#define TMPPAGE		(USTACKTOP)
#define TMPPAGETOP	(USTACKTOP+BY2PG)

int init_pthread_stack(int envid, void * (*start_routine) (void *), void * arg, u_int * init_esp) {
    if (syscall_mem_alloc(0, TMPPAGE, PTE_V|PTE_R) < 0) return -PTH_AGAIN;

    *((u_int *) (TMPPAGETOP - 8)) = (u_int) start_routine;
    *((u_int *) (TMPPAGETOP - 4)) = (u_int) arg;

    *init_esp = USTACKTOP - 8;

    if (syscall_mem_map(0, TMPPAGE, envid, USTACKTOP-BY2PG, PTE_V|PTE_R) < 0)
        goto error;
    if (syscall_mem_unmap(0, TMPPAGE) < 0)
        goto error;

    // writef("finish init_pthread_stack successfully.\n");
    return 0;

    error:
    syscall_mem_unmap(0, TMPPAGE);
    return -PTH_AGAIN;
}

void threadmain(void * (*start_routine) (void *), void * arg) {
    start_routine(arg);
    // writef("syscall_pthread_finish in threadmain!!!\n");
    // writef("threadmain\n");
    syscall_set_pth_status(0, PTH_FINISHED);
    syscall_env_destroy(0);
}

int pthread_create(pthread_t * thread, const pthread_attr_t * attr, void * (*start_routine) (void *), void * arg) {
    // writef("sys_pthread_create %08x %08x\n", (u_int) start_routine, (u_int) arg);
    *thread = syscall_pthread_alloc();
    extern struct Pthread *pthreads;
    int pthenv_envid = *thread;
    u_int esp;
    if (init_pthread_stack(pthenv_envid, start_routine, arg, &esp) < 0) return -PTH_AGAIN;
    struct Trapframe * tf;
    tf = &(envs[ENVX(pthenv_envid)].env_tf);
    tf->cp0_epc = tf->pc = threadmain;
    writef("threadmain : %x\n", tf->pc);
    tf->regs[29] = esp;
    tf->regs[4] = (u_int) start_routine;
    tf->regs[5] = (u_int) arg;
    syscall_set_env_status(pthenv_envid, ENV_RUNNABLE);
    // writef("pthread_create\n");
    syscall_set_pth_status(pthenv_envid, PTH_RUNNING);
    // writef("finish pthread_create in user\n");
    return 0;
}

void pthread_exit(void *ret_val) {
    // writef("pthread_exit %d\n", ret_val);
    syscall_set_pth_retval(0, ret_val);
    syscall_set_pth_status(0, PTH_FINISHED);
    syscall_env_destroy(0);
}

int pthread_cancel(pthread_t thread) {
    // writef("pthread_cancel\n");
    syscall_set_pth_status(thread, PTH_FREE);
    syscall_env_destroy(thread);
}

int pthread_join(pthread_t thread, void **ret_val) {
    while (syscall_read_pth_status(thread) != PTH_FINISHED) {
        syscall_yield();
    }
    // writef("pthread_join\n");
    syscall_set_pth_status(thread, PTH_FREE);
    if (ret_val != NULL) {
        *ret_val = syscall_read_pth_retval(thread);
    }
    return 0;
}
