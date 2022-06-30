//
// Created by root on 6/29/22.
//
#include <types.h>

#ifndef SCSE_PTHREAD_H
#define SCSE_PTHREAD_H

struct Pthread {
    u_int pth_status;
    void * pth_retval;
};

extern struct Pthread *pthread;

#define NULL ((void *) 0)

typedef u_int pthread_t;
typedef int pthread_attr_t;

int pthread_create(pthread_t * thread, const pthread_attr_t * attr, void * (*start_routine) (void *), void * arg, u_int threadmain);
void pthread_exit(void *ret_val);
int pthread_cancel(pthread_t thread);
int pthread_join(pthread_t thread, void **ret_val);

#define PTH_RUNNING 1
#define PTH_FINISHED 2
#define PTH_FREE 0

#define PTH_AGAIN 1
#define PTH_INVAL 2
#define PTH_SRCH 3
#endif //SCSE_PTHREAD_H
