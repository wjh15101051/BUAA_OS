#include <pthread.h>
#include <mmu.h>

int pthread_create(pthread_t * thread, const pthread_attr_t * attr, void * (*start_routine) (void *), void * arg);
void pthread_exit(void *ret_val);
int pthread_cancel(pthread_t thread);
int pthread_join(pthread_t thread, void **ret_val);