#include "lib.h"
#include <pthread.h>
#include <printf.h>
#include <mmu.h>
#include <env.h>

int init_pthread_stack(int envid, void * (*start_routine) (void *), void * arg, u_int * init_esp) {
    int TMPPAGE, TMPPAGETOP, i;
    Pde* vpde = (Pde *) (UVPT+(UVPT>>12)*4);
    Pte* vpte = (Pte *) UVPT;
    for (i = 2 * PDMAP; i < USTACKTOP - BY2PG; i += BY2PG) {
        if (!(vpde[i >> PDSHIFT] & PTE_V) || !(vpte[i >> PGSHIFT] & PTE_V)) {
            TMPPAGE = i;
            break;
        }
    }
    TMPPAGETOP = TMPPAGE + BY2PG;

    if (syscall_mem_alloc(0, TMPPAGE, PTE_V|PTE_R) < 0) return -PTH_AGAIN;

    *((u_int *) (TMPPAGETOP - 8)) = (u_int) start_routine;
    *((u_int *) (TMPPAGETOP - 4)) = (u_int) arg;

    *init_esp = TMPPAGETOP - 8;

    return 0;
}

extern void __asm_pgfault_handler(void);

int init_pthread_xstack(int envid) {
    int TMPPAGE, i;
    Pde* vpde = (Pde *) (UVPT+(UVPT>>12)*4);
    Pte* vpte = (Pte *) UVPT;
    for (i = 2 * PDMAP; i < USTACKTOP - BY2PG; i += BY2PG) {
        if (!(vpde[i >> PDSHIFT] & PTE_V) || !(vpte[i >> PGSHIFT] & PTE_V)) {
            TMPPAGE = i;
            break;
        }
    }

    if (syscall_mem_alloc(0, TMPPAGE, PTE_V|PTE_R) < 0) return -PTH_AGAIN;

    if (syscall_set_pgfault_handler(envid, __asm_pgfault_handler, TMPPAGE) < 0) return -PTH_AGAIN;

    return 0;
}

void threadmain(void * (*start_routine) (void *), void * arg) {
    start_routine(arg);
    // writef("syscall_pthread_finish in threadmain!!!\n");
    // writef("threadmain\n");
    syscall_set_pth_status(0, PTH_FINISHED);
    syscall_pthread_destroy(0);
}

int pthread_create(pthread_t * thread, const pthread_attr_t * attr, void * (*start_routine) (void *), void * arg) {
    // writef("sys_pthread_create %08x %08x\n", (u_int) start_routine, (u_int) arg);
    *thread = syscall_pthread_alloc();
    extern struct Pthread *pthreads;
    int pthenv_envid = *thread;
    u_int esp;
    if (init_pthread_stack(pthenv_envid, start_routine, arg, &esp) < 0) return -PTH_AGAIN;
    if (init_pthread_xstack(pthenv_envid) < 0) return -PTH_AGAIN;
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
    syscall_pthread_destroy(0);
}

int pthread_cancel(pthread_t thread) {
    // writef("pthread_cancel\n");
    while (syscall_read_pth_status(thread) != PTH_RUNNING) {
        syscall_yield();
    }
    syscall_set_pth_status(thread, PTH_FREE);
    syscall_pthread_destroy(thread);
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
