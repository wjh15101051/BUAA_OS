#include <printf.h>
#include <pthread.h>
#include <env.h>
#include <mmu.h>
#include <queue.h>
#include <pmap.h>
#include <printf.h>
#include <sched.h>

extern struct Env *curenv;
extern struct Env_list env_free_list;

struct Pthread *pthreads;

#define MAXTREAD 20
int thread_count = 0;

int pthread_mem_alloc(struct Env *e, u_int va, u_int perm) {
    struct Page *ppage;
    if ( (va >= UTOP) || ((perm & PTE_V) == 0) || ((perm & PTE_COW) != 0) ) return -PTH_AGAIN;
    if (page_alloc(&ppage) < 0) return -PTH_AGAIN;
    if (page_insert(e->env_pgdir, ppage, va, perm) < 0) return -PTH_AGAIN;
    return 0;
}

int pthread_mem_map(struct Env * srce, u_int srcva, struct Env * dste, u_int dstva, u_int perm) {
    u_int round_srcva, round_dstva;
    struct Page *ppage;
    Pte *ppte;

    ppage = NULL;
    round_srcva = ROUNDDOWN(srcva, BY2PG);
    round_dstva = ROUNDDOWN(dstva, BY2PG);

    //your code here
    if ((perm & PTE_V) == 0) return -PTH_AGAIN;
    ppage = page_lookup(srce -> env_pgdir, round_srcva, &ppte);
    if (ppage == NULL) return -PTH_AGAIN;
    if (((*ppte & PTE_R) == 0) && ((perm & PTE_R) != 0)) return -PTH_AGAIN;
    if (page_insert(dste -> env_pgdir, ppage, round_dstva, perm) < 0) return -PTH_AGAIN;
    return 0;
}

int pthread_mem_unmap(struct Env * e, u_int va) {
    page_remove(e -> env_pgdir, va);
    return 0;
}

extern Pde *boot_pgdir;

int pthread_setup_vm(struct Env *e) {
    int i;
    struct Page *p = NULL;
    if (page_alloc(&p) < 0) return -PTH_AGAIN;
    ++p->pp_ref;
    Pde * pgdir;
    pgdir = (Pde *) page2kva(p);
    e->env_pgdir = pgdir;
    e->env_cr3 = PADDR(pgdir);
    Pde* vpde = (Pde *) (UVPT+(UVPT>>12)*4);
    Pte* vpte = (Pte *) UVPT;
    for (i = 0; i < USTACKTOP - BY2PG; i += BY2PG) {
        if ((vpde[i >> PDSHIFT] & PTE_V) && (vpte[i >> PGSHIFT] & PTE_V)) {
            pthread_mem_map(curenv, i, e, i, PTE_V | PTE_R);
        }
    }
    for (i = PDX(USTACKTOP - BY2PG); i < PDX(UTOP); ++i) {
        pgdir[i] = 0;
    }
    for (i = PDX(UTOP); i < PDX(ULIM); ++i) {
        if (i == PDX(UVPT)) continue;
        pgdir[i] = boot_pgdir[i];
    }
    pgdir[PDX(UVPT)] = e->env_cr3 | PTE_V;
    if (pthread_mem_alloc(e, UXSTACKTOP - BY2PG, PTE_V | PTE_R) < 0) return -PTH_AGAIN;
    return 0;
}

int pthread_alloc(struct Env **new) {
    printf("pthread_alloc\n");
    struct Env *e;
    if (thread_count == MAXTREAD) {
        return -PTH_AGAIN;
    }
    ++thread_count;
    if (LIST_EMPTY(&env_free_list)) {
        *new = NULL;
        return -PTH_AGAIN;
    }
    while (1) {
        e = LIST_FIRST(&env_free_list);
        LIST_REMOVE(e, env_link);
        int x = ENVX(e->env_id);
        u_int st = ((struct Pthread *) (pthreads + x))->pth_status;
        if (st != PTH_FREE) {
            LIST_INSERT_TAIL(&env_free_list, e, env_link);
        } else {
            break;
        }
    }
    e = LIST_FIRST(&env_free_list);

    if (pthread_setup_vm(e) < 0) return -PTH_AGAIN;

    e->env_id = mkenvid(e);
    printf("thread env id : %d\n", e->env_id);
    e->env_status = ENV_RUNNABLE;
    e->env_parent_id = 0;
    e->env_runs = 0;

    e->env_tf.cp0_status = 0x1000100c;
    e->env_tf.regs[29] = USTACKTOP;

    *new = e;
    return 0;
}

#define TMPPAGE		(USTACKTOP)
#define TMPPAGETOP	(USTACKTOP+BY2PG)

int init_pthread_stack(struct Env *e, void * (*start_routine) (void *), void * arg, u_int * init_esp) {
    if (pthread_mem_alloc(curenv, TMPPAGE, PTE_V|PTE_R) < 0) return -PTH_AGAIN;

    *((u_int *) (TMPPAGETOP - 8)) = (u_int) start_routine;
    *((u_int *) (TMPPAGETOP - 4)) = (u_int) arg;

	*init_esp = USTACKTOP - 8;

    printf("%08x %08x\n", curenv->env_id, e->env_id);
    if (pthread_mem_map(curenv, TMPPAGE, e, USTACKTOP-BY2PG, PTE_V|PTE_R) < 0)
        goto error;
    if (pthread_mem_unmap(curenv, TMPPAGE) < 0)
        goto error;

    printf("finish init_pthread_stack successfully.\n");
    return 0;

    error:
    pthread_mem_unmap(curenv, TMPPAGE);
    return -PTH_AGAIN;
}

int pthread_set_env_status(struct Env * e, u_int status) {
    if (status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE && status != ENV_FREE) return -PTH_AGAIN;
    if (status == ENV_FREE) {
        env_destroy(e);
    } else {
        e -> env_status = status;
    }
    return 0;
}

int pthread_create(pthread_t * thread, const pthread_attr_t * attr, void * (*start_routine) (void *), void * arg, u_int threadmain) {
    printf("sys_pthread_create %08x %08x\n", (u_int) start_routine, (u_int) arg);
    struct Env * pthenv;
    if (pthread_alloc(&pthenv) < 0) return -PTH_AGAIN;
    *thread = pthenv->env_id;
    pthenv->env_pri = 1;
    u_int esp;
    if (init_pthread_stack(pthenv, start_routine, arg, &esp) < 0) return -PTH_AGAIN;
    struct Trapframe * tf;
    tf = &pthenv->env_tf;
    tf->cp0_epc = tf->pc = threadmain;
    printf("threadmain : %x\n", tf->pc);
    tf->regs[29] = esp;
    tf->regs[4] = (u_int) start_routine;
    tf->regs[5] = (u_int) arg;
    pthread_set_env_status(pthenv, ENV_RUNNABLE);
    LIST_INSERT_HEAD(&env_sched_list[0], pthenv, env_sched_link);
    printf("finish pthread_create in kernel\n");
    struct Pthread *pth;
    pth = pthreads + (u_int) (pthenv - envs);
    pth->pth_status = PTH_RUNNING;
    return 0;
}

void pthread_exit(void *retval) {
    printf("sys_pthread_exit : %d\n", (int) retval);
    struct Pthread *pth;
    pth = pthreads + (u_int) (curenv - envs);
    env_destroy(curenv);
    pth->pth_status = PTH_FINISHED;
    pth->pth_retval = retval;
}

int pthread_cancel(pthread_t thread) {
    printf("sys_pthread_cancel\n");
    --thread_count;
    struct Pthread *pth;
    pth = pthreads + ENVX(thread);
    pth->pth_status = PTH_FREE;
    struct Env *e;
    if (envid2env(thread, &e, 0) < 0) return -PTH_SRCH;
    env_destroy(e);
    return 0;
}

int pthread_join(pthread_t thread, void **retval) {
    printf("sys_pthread_join\n");
    --thread_count;
    struct Pthread *pth;
    pth = pthreads + ENVX(thread);
    extern char *KERNEL_SP;
    while (pth->pth_status != PTH_FINISHED);
    pth->pth_status = PTH_FREE;
    *retval = pth->pth_retval;
    printf("^^^^^^^^^^^^^^^^^ %08x ^^^^^^^^^^^^^^\n", pth->pth_retval);
    return 0;
}
