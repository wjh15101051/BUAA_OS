//
// Created by root on 6/29/22.
//
#include <types.h>

#ifndef SCSE_PTHREAD_H
#define SCSE_PTHREAD_H

struct Pthread {
    u_int pth_status;
    void * pth_retval;
    u_int env_id;
};

extern struct Pthread *pthreads;

#define NULL ((void *) 0)

typedef u_int pthread_t;
typedef int pthread_attr_t;

#define PTH_RUNNING 1
#define PTH_FINISHED 2
#define PTH_FREE 0

#define PTH_AGAIN 1
#define PTH_INVAL 2
#define PTH_SRCH 3
#endif //SCSE_PTHREAD_H
