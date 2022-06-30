#include <printf.h>
#include <pthread.h>
#include <env.h>
#include <mmu.h>
#include <queue.h>
#include <pmap.h>
#include <sched.h>

struct Pthread *pthreads = NULL;

extern struct Env *curenv;
extern struct Env_list env_free_list;

#define MAXTREAD 20

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
    // printf("pthread_alloc\n");
    struct Env *e;
    if (LIST_EMPTY(&env_free_list)) {
        *new = NULL;
        return -PTH_AGAIN;
    }
    while (1) {
        e = LIST_FIRST(&env_free_list);
        LIST_REMOVE(e, env_link);
        int x = e - envs;
        u_int st = ((struct Pthread *) (pthreads + x))->pth_status;
        if (st != PTH_FREE) {
            LIST_INSERT_TAIL(&env_free_list, e, env_link);
        } else {
            break;
        }
    }

    if (pthread_setup_vm(e) < 0) return -PTH_AGAIN;

    e->env_id = mkenvid(e);
    // printf("thread env id : %d\n", e->env_id);
    e->env_status = ENV_NOT_RUNNABLE;
    e->env_parent_id = 0;
    e->env_runs = 0;

    e->env_tf.cp0_status = 0x1000100c;
    e->env_tf.regs[29] = USTACKTOP;

    *new = e;

    LIST_INSERT_HEAD(&env_sched_list[0], e, env_sched_link);
    u_int cur_env_id = (pthreads + (u_int) (curenv - envs))->env_id;
    if (cur_env_id == 0) {
        (pthreads + (u_int) (e - envs))->env_id = curenv->env_id;
    } else {
        (pthreads + (u_int) (e - envs))->env_id = cur_env_id;
    }
    return 0;
}
